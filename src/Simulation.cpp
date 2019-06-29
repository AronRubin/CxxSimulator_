// CxxSimulator.cpp : Defines the entry point for the application.
//

#include <CxxSimulator/Simulator.h>
#include "Simulation_p.h"
#include "Instance_p.h"

#include <map>
#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <algorithm>
#include <deque>
#include <list>
#include <any>
#include <variant>

namespace sim {

struct SimEvent {
  enum class Type {
    STATE_CHANGE,
    SPAWN_INSTANCE,
    SPAWN_ACTIVITY,
    RESUME_ACTIVITY,
    SPAWN_PAD,
    PAD_SEND
  };

  SimEvent( Type type,
      const Clock::time_point &time,
      const std::string &spec,
      const std::string &name,
      const std::string &owner = {},
      std::any payload = {} ) :
      type{ type },
      time{ time },
      spec{ spec },
      name{ name },
      owner{ owner },
      payload{ payload } {}
  ~SimEvent() = default;
  SimEvent( SimEvent & ) = default;
  SimEvent &operator=( SimEvent & ) = default;
  SimEvent( SimEvent && ) = default;
  SimEvent &operator=( SimEvent && ) = default;

  Type type;
  Clock::time_point time;
  std::string spec;
  std::string name;
  std::string owner;
  std::any payload;

  friend bool operator<( const SimEvent &eva, const SimEvent &evb ) {
    return eva.time < evb.time;
  }
  friend bool operator>( const SimEvent &eva, const SimEvent &evb ) {
    return eva.time > evb.time;
  }
};

struct WaitingActivity {
  using PromiseVariant = std::variant<std::promise<bool>, std::promise<std::any>>;

  WaitingActivity( const Clock::time_point &time = {} ) :
      promise{ std::in_place_index<0> },
      time{ time } {}
  WaitingActivity( const std::string &signal_name, const Clock::time_point &time ) :
      promise{ std::in_place_index<1> },
      time{ time },
      signal_name{ signal_name } {}
  PromiseVariant promise;
  Clock::time_point time;
  std::string signal_name;
};

struct Simulation::Impl {
  Impl( Simulation &simulation ) : m_simulation{ simulation } {}
  ~Impl() = default;
  Impl( Impl &&other ) = default;
  Impl &operator=( Impl &&other ) = default;
  
  Simulation &m_simulation; // Simulation owns Simulation::Impl
  Clock::time_point m_simtime;
  State m_state = State::INIT;
  State m_pending_state = State::INIT;
  PropertyList m_parameters;
  std::map<std::string, std::shared_ptr<Instance>> m_instances;
  std::deque<SimEvent> m_events;
  std::map<std::shared_ptr<Activity>, WaitingActivity> m_waiting_activities;
  std::thread m_worker;
  std::mutex m_state_mut;
  std::condition_variable m_state_cnd;

  acpp::void_result<> insertSpawnInstance(
      const std::string &model,
      const std::string &name,
      const PropertyList &parameters,
      const Clock::time_point &time );

  acpp::void_result<> insertSpawnActivity(
      const std::string &spec,
      const std::string &name,
      const std::string &instance,
      const Clock::time_point &time );

  std::future<bool> insertResumeActivity(
      std::shared_ptr<Activity> activity,
      const Clock::time_point &time );

  std::future<bool> activityWaitOn(
      std::shared_ptr<Activity> activity,
      const std::string &signal_name,
      const Clock::time_point &time = {} );

  void handleStateChange( const SimEvent &event );
  void handleSpawnInstance( const SimEvent &event );
  void handleSpawnActivity( const SimEvent &event );
  void handleResumeActivity( const SimEvent &event );
  void handleSpawnPad( const SimEvent &event );
  void handlePadSend( const SimEvent &event );

  void setState( const Simulation::State &state );
  void step();
  void workerFunc();
};

Simulation::Simulation() : impl( new Impl{ *this } ) {}
Simulation::~Simulation() = default;
Simulation::Simulation( Simulation &&other ) = default;
Simulation &Simulation::operator=( Simulation &&other ) = default;

acpp::void_result<> Simulation::spawnInstance(
    const std::string &model,
    const std::string &name,
    const PropertyList &parameters,
    const Clock::time_point &time ) {
  return impl->insertSpawnInstance( name, model, parameters, time );
}

acpp::void_result<> Simulation::spawnActivity(
    const std::string &spec_name,
    const std::string &name,
    const std::string &instance,
    const Clock::time_point &time ) {
  return impl->insertSpawnActivity( spec_name, name, instance, time );
}

acpp::void_result<> Simulation::Impl::insertSpawnInstance(
    const std::string &name,
    const std::string &model,
    const PropertyList &parameters,
    const Clock::time_point &time ) {
  auto iiter = m_instances.find( name );
  if ( iiter != m_instances.end() ) {
    return {{}, "instance not unique"};
  }
  // TODO lock here
  // check if this is pending spawn
  auto eiter = std::find_if( m_events.begin(), m_events.end(), [name]( const SimEvent &event ) {
    return event.type == SimEvent::Type::SPAWN_INSTANCE && event.name == name;
  } );
  if ( eiter != m_events.end() ) {
    return {{}, "instance not unique"};
  }

  Clock::time_point event_time = time;
  if ( time.time_since_epoch() == Clock::duration::zero() ) {
    event_time = m_simtime;
  }
  // TODO fix parameter passing
  m_events.emplace_back( SimEvent::Type::SPAWN_INSTANCE, event_time, model, name, model /*, parameters */ );
  std::push_heap( m_events.begin(), m_events.end(), std::greater<SimEvent>{} );

  return {};
}

acpp::void_result<> Simulation::Impl::insertSpawnActivity(
    const std::string &spec_name,
    const std::string &name,
    const std::string &instance,
    const Clock::time_point &time ) {
  auto iiter = m_instances.find( instance );
  if ( iiter == m_instances.end() ) {
    // TODO allow if there is a spawn instance of the right name before time
    return {{}, "instance not found"};
  }
  // TODO lock here
  Clock::time_point event_time = time;
  if ( time.time_since_epoch() == Clock::duration::zero() ) {
    event_time = m_simtime;
  }
  m_events.emplace_back( SimEvent::Type::SPAWN_ACTIVITY, event_time, spec_name, name, instance );
  std::push_heap( m_events.begin(), m_events.end(), std::greater<SimEvent>{} );

  return {};
}

std::future<bool> Simulation::Impl::insertResumeActivity(
    std::shared_ptr<Activity> activity,
    const Clock::time_point &time ) {
  // TODO lock here
  // TODO check that event_time is >= simtime
  Clock::time_point event_time = time;
  if ( time.time_since_epoch() == Clock::duration::zero() ) {
    event_time = m_simtime;
  }
  m_events.emplace_back(
      SimEvent::Type::RESUME_ACTIVITY,
      event_time,
      activity->name(),
      activity->owner()->name() );
  std::push_heap( m_events.begin(), m_events.end(), std::greater<SimEvent>{} );
  auto &wact = m_waiting_activities[activity];
  wact = {event_time};
  auto &promise = std::get<0>( wact.promise );

  // it seems copy elision is not assumed here
  return std::move( promise.get_future() );
}

std::future<bool> Simulation::Private::insertResumeActivity(
    std::shared_ptr<Simulation> simulation,
    std::shared_ptr<Activity> act,
    const Clock::time_point &time ) {
  return simulation->impl->insertResumeActivity( act, time );
}

std::future<bool> Simulation::Impl::activityWaitOn(
    std::shared_ptr<Activity> activity,
    const std::string &signal_name,
    const Clock::time_point &time ) {
  // TODO lock here
  // TODO check that event_time is >= simtime
  if ( time.time_since_epoch() != Clock::duration::zero() ) {
    m_events.emplace_back(
        SimEvent::Type::RESUME_ACTIVITY,
        time,
        std::string {},
        activity->name(),
        activity->owner()->name() );
    std::push_heap( m_events.begin(), m_events.end(), std::greater<SimEvent>{} );
  }
  auto &wact = m_waiting_activities[activity];
  wact = { signal_name, time };
  auto &promise = std::get<0>( wact.promise );

  // it seems copy elision is not assumed here
  return std::move( promise.get_future() );
}

std::future<bool> Simulation::Private::activityWaitOn(
    std::shared_ptr<Simulation> simulation,
    std::shared_ptr<Activity> activity,
    const std::string &signal_name,
    const Clock::time_point &time ) {
  if ( !simulation || !activity ) {
    return {}; // TODO return error
  }
  return simulation->impl->activityWaitOn( activity, signal_name, time );
}

// global parameters
acpp::void_result<> Simulation::setParameter(
    const std::string &name,
    const acpp::unstructured_value &value ) {
  impl->m_parameters.emplace( name, value );
  return {};
}

acpp::unstructured_value Simulation::parameter( const std::string &name ) const {
  auto iter = impl->m_parameters.find( name );
  if ( iter == impl->m_parameters.end() ) {
    return {};
  }
  return iter->second;
}

Clock::time_point Simulation::simtime() const {
  return impl->m_simtime;
}

Simulation::State Simulation::state() const {
  return impl->m_state;
}

Simulation::State Simulation::state( State &pending ) const {
  pending = impl->m_pending_state;
  return impl->m_state;
}

acpp::void_result<> Simulation::setState( const Simulation::State &state ) {
  impl->setState( state );
  return {};
}

void Simulation::Impl::setState( const Simulation::State &state ) {
  std::unique_lock<std::mutex> state_lock( m_state_mut );
  if ( m_state == state ) {
    return;
  }
  m_pending_state = state;
  m_state = state;
  m_state_cnd.notify_all();
}

void Simulation::Impl::handleStateChange( const SimEvent &event ) {
  // TODO STUB
}

void Simulation::Impl::handleSpawnInstance( const SimEvent &event ) {
  auto model = Simulator::getInstance().model( event.spec );
  if ( !model ) {
    return;
  }
  // TODO check name unique
  auto rv = m_instances.emplace(
      event.name,
      std::make_shared<Instance>(
          m_simulation.shared_from_this(),
          model,
          event.name,
          PropertyList {} ) );
}

void Simulation::Impl::handleSpawnActivity( const SimEvent &event ) {
  auto model = Simulator::getInstance().model( event.spec );
  if ( !model ) {
    return;
  }
  auto iiter = m_instances.find( event.owner );
  if ( iiter == m_instances.end() ) {
    return;
  }
  Instance::Private::insertActivity( iiter->second, event.spec, event.name );
}

void Simulation::Impl::handleResumeActivity( const SimEvent &event ) {
  auto iiter = m_instances.find( event.owner );
  if ( iiter == m_instances.end() ) {
    return;
  }
  auto activity = iiter->second->activity( event.name );
  if ( !activity ) {
    return;
  }
  auto witer = m_waiting_activities.find( activity );
  if ( witer == m_waiting_activities.end() ) {
    return;
  }
  std::visit(
      [&]( auto &promise ) {
        using Ptype = std::decay_t<decltype( promise )>;
        if constexpr (std::is_same_v<Ptype, std::promise<bool>>) {
          promise.set_value( true );
        } else {
          promise.set_value( event.payload );
        }
      },
      witer->second.promise );
  m_waiting_activities.erase( witer );
}

void Simulation::Impl::handleSpawnPad( const SimEvent &event ) {
}

void Simulation::Impl::handlePadSend( const SimEvent &event ) {
}

void Simulation::Impl::step() {
  if ( m_events.empty() ) {
    setState( State::DONE );
  }
  std::pop_heap( m_events.begin(), m_events.end(), std::greater<SimEvent>{} );
  auto event = m_events.back();
  m_events.pop_back();

  if ( event.time > m_simtime ) {
    m_simtime = event.time;
  }

  switch ( event.type ) {
  case SimEvent::Type::STATE_CHANGE:
    handleStateChange( event );
    break;
  case SimEvent::Type::SPAWN_INSTANCE:
    handleSpawnInstance( event );
    break;
  case SimEvent::Type::SPAWN_ACTIVITY:
    handleSpawnActivity( event );
    break;
  case SimEvent::Type::RESUME_ACTIVITY:
    handleResumeActivity( event );
    break;
  case SimEvent::Type::SPAWN_PAD:
    handleSpawnPad( event );
    break;
  case SimEvent::Type::PAD_SEND:
    handlePadSend( event );
    break;
  }
}

void Simulation::Impl::workerFunc() {
  std::unique_lock<std::mutex> state_lock( m_state_mut );
  
  m_state_cnd.wait( state_lock, [this]() { return m_state != State::INIT; } );
  while ( m_state != State::DONE ) {
    m_state_cnd.wait( state_lock, [this]() { return m_state != State::PAUSE; } );
    if (m_state == State::RUN) {
      state_lock.unlock();
      step();
      state_lock.lock();
    }
  }
}

}  // namespace sim
