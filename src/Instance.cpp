// CxxSimulator.cpp : Defines the entry point for the application.
//

#include <CxxSimulator/Simulator.h>
#include <CxxSimulator/Model.h>
#include "Simulation_p.h"
#include "Instance_p.h"

#include <map>
#include <string>
#include <memory>
#include <queue>
#include <shared_mutex>
#include <random>

namespace sim {

struct Instance::Impl {
  Impl(
      Instance &instance,
      std::shared_ptr<Simulation> simulation,
      std::shared_ptr<Model> model,
      const std::string &name,
      const PropertyList &parameters ) :
      m_instance{ instance },
      m_simulation{ simulation },
      m_model{ model },
      m_name{ name },
      m_parameters{ parameters } {
    if ( m_name.empty() ) {
      throw "name not supplied";
    }
    if( !m_simulation ) {
      throw "simulation not supplied";
    }
    if( !m_model ) {
      throw "model not supplied";
    }
    makeStartActivity();
  }
  
  ~Impl() = default;
  Impl( Impl &&other ) noexcept = default;
  Impl &operator=( Impl &&other ) noexcept = default;

  void makeStartActivity(); // can throw on error because making invariant
  acpp::void_result<> spawnActivity( const std::string spec_name, const std::string &name, Clock::duration delay );

  Instance &m_instance; // Instance owns Instance::Impl
  std::shared_ptr<Simulation> m_simulation;
  std::shared_ptr<Model> m_model;
  std::string m_name;
  PropertyList m_parameters;
  std::unordered_map<std::string, std::shared_ptr<Activity>> m_activities;
#if ACPP_LESSON > 3
  std::unordered_map<std::string, std::shared_ptr<Pad>> m_pads;
#endif // ACPP_LESSON > 3
};

Instance::~Instance() = default;
Instance::Instance( Instance &&other ) noexcept = default;
Instance &Instance::operator=( Instance &&other ) noexcept = default;

Instance::Instance(
    std::shared_ptr<Simulation> sim,
    std::shared_ptr<Model> model,
    const std::string &name,
    const PropertyList &parameters ) :
    impl( new Impl{ *this, sim, model, name, parameters } ) {
}

std::string Instance::name() const {
  return impl->m_name;
}

std::shared_ptr<Simulation> Instance::owner() const {
  return impl->m_simulation;
}

std::shared_ptr<Model> Instance::model() const {
  return impl->m_model;
}

#if ACPP_LESSON > 3
std::shared_ptr<Pad> Instance::pad( const std::string &name ) const {
  auto iter = impl->m_pads.find( name );
  if ( iter == impl->m_pads.end() ) {
    return {};
  }
  return iter->second;
}
#endif // ACPP_LESSON > 3

acpp::unstructured_value Instance::parameter( const std::string &name ) const {
  auto iter = impl->m_parameters.find( name );
  if ( iter == impl->m_parameters.end() ) {
    return {};
  }
  return iter->second;
}

std::vector<std::shared_ptr<Activity>> Instance::activities() const {
  std::vector<std::shared_ptr<Activity>> activities;
  for ( const auto &activityent : impl->m_activities ) {
    activities.push_back( activityent.second );
  }
  return activities;
}

std::shared_ptr<Activity> Instance::activity( const std::string &name ) const {
  auto iter = impl->m_activities.find( name );
  if ( iter == impl->m_activities.end() ) {
    return {};
  }
  return iter->second;
}

void Instance::Impl::makeStartActivity() {
  ActivitySpec spec( "start", ActivitySpec::Type::plain );
  auto activity = std::make_shared<Activity>( m_instance.shared_from_this(), spec, "start" );
  if (!activity) {
    throw "could not create start activity";
  }
  auto rv = m_activities.emplace( "start", activity );
  if (!rv.second) {
    throw "could not insert start activity";
  }
}

std::shared_ptr<Activity> Instance::addActivity( const std::string &spec_name, const std::string &name ) {
  auto spec = impl->m_model->activity( spec_name );
  if (spec.type == ActivitySpec::Type::undefined || spec.name.empty() ) {
    return {};
  }
  if (this->activity( name )) {
    return {};
  }
  auto activity = makeActivity( spec, name );
  if (!activity) {
    return {};
  }
  auto rv = impl->m_activities.emplace( name, activity );
  return activity;
}

std::shared_ptr<Activity> Instance::makeActivity( const ActivitySpec &spec, const std::string &name ) {
  return std::make_shared<Activity>( shared_from_this(), spec, name );
}

acpp::void_result<> Instance::spawnActivity( const std::string spec_name, const std::string &name, Clock::duration delay ) {
  return impl->spawnActivity( spec_name, name, delay );
}

struct Activity::Impl {
  Impl(
      Activity &activity,
      std::shared_ptr<Instance> instance,
      const ActivitySpec &spec,
      const std::string &name,
      const Func &func ) :
      m_activity{ activity },
      m_instance{ instance },
      m_spec{ spec },
      m_name{ name },
      m_func{ func } {
    if ( name.empty() ) {
      throw "name not supplied";
    }
  }
  Activity &m_activity; // Activity owns Activity::Impl
  std::weak_ptr<Instance> m_instance;
  ActivitySpec m_spec;
  std::string m_name;
  Func m_func;
#if ACPP_LESSON > 4
  State m_state = State::init;
  std::thread m_worker;
  std::mutex m_state_mut;
  std::condition_variable m_state_cnd;
#endif // ACPP_LESSON > 4

  void invoke( const std::string &source, const std::any &payload );

#if ACPP_LESSON > 3
  acpp::value_result<std::any> padReceive( const std::string &pad_name, sim::Clock::time_point time, const std::string &activity_name );
  bool padSend( const std::string &pad_name, const std::any &payload, const std::string &activity_name );
#endif

#if ACPP_LESSON > 4
  void waitUntil( const sim::Clock::time_point &time );
  acpp::value_result<std::any> padReceive( const std::string &pad_name, sim::Clock::time_point time );
  bool padSend( const std::string &pad_name, const std::any &payload, bool block = false );
  void workerFunc();
#endif
};

Activity::Activity(
  std::shared_ptr<Instance> instance,
  const ActivitySpec &spec,
  const std::string &name,
  const Func &func ) :
    impl( new Impl{ *this, instance, spec, name, func } ) {
}

Activity::~Activity() noexcept = default;
Activity::Activity( Activity &&other ) noexcept = default;
Activity &Activity::operator=( Activity &&other ) noexcept = default;

ActivitySpec Activity::spec() const {
  return impl->m_spec;
}

void Activity::Impl::invoke( const std::string &source, const std::any &payload ) {
  if (!m_func) {
    return;
  }
  std::invoke( m_func, *m_instance.lock(), m_activity, source, payload );
}

void Activity::invoke( const std::string &source, const std::any &payload ) {
  impl->invoke( source, payload );
}

#if ACPP_LESSON > 4
Activity::State Activity::state() const {
  return impl->m_state;
}
#endif // ACPP_LESSON > 4

std::shared_ptr<Instance> Activity::owner() const {
  return impl->m_instance.lock();
}

std::string Activity::name() const {
  return impl->m_spec.name;
}


#if ACPP_LESSON > 4
void Activity::Impl::workerFunc() {
  if ( !m_spec.function ) {
    return;
  }
  std::unique_lock<std::mutex> state_lock( m_state_mut );
  m_state_cnd.wait( state_lock, [this]() { return m_state != State::init && m_state != State::pause; } );
  if ( m_state == State::run ) {
    state_lock.unlock();
    std::invoke( m_func, *m_instance.lock(), m_activity, {}, {} );
  }
}
#endif // ACPP_LESSON > 4

#if ACPP_LESSON > 4
void Activity::Impl::waitUntil( const sim::Clock::time_point &time ) {
  // TODO lock
  if ( m_state != State::RUN ) {
    return;
  }
  m_state = State::PAUSE;
  auto future = Simulation::Private::insertResumeActivity(
      m_instance.lock()->owner(),
      m_activity.shared_from_this(),
      time );
  future.get();
}
#endif // ACPP_LESSON > 4

#if ACPP_LESSON > 4
void Activity::waitUntil( const sim::Clock::time_point &time ) {
  impl->waitUntil( time );
}
void Activity::waitFor( sim::Clock::duration dur ) {
  auto instance = impl->m_instance.lock();
  waitUntil( instance->owner()->simtime() + dur );
}
#endif // ACPP_LESSON > 4

#if ACPP_LESSON > 3
acpp::value_result<std::any> Activity::Impl::padReceive( const std::string &pad_name, sim::Clock::time_point time, const std::string &activity_name ) {
  auto instance = m_instance.lock();
  auto pad = m_instance.lock()->pad( pad_name );
  if ( !pad ) {
    return {{}, "no pad: " + pad_name};
  }
  
  if ( pad->available() > 0 ) {
    return Pad::Private::pull( pad );
  }
  
#if ACPP_LESSON > 4
  // TODO lock state
  if ( m_state != State::RUN ) {
    return { {}, "already waiting" }; // Activity already waiting
  }
  m_state = State::PAUSE;
  auto future = Simulation::Private::activityPadReceive(
      instance->owner(),
      m_activity.shared_from_this(),
      pad_name,
      time );
  auto rv = future.get();
  if (!rv) {
    return { {}, "receive canceled" };
  }
#endif // ACPP_LESSON > 4

  return Pad::Private::pull( pad );
}
#endif // ACPP_LESSON > 3

#if ACPP_LESSON > 4
acpp::value_result<std::any> Activity::Impl::padReceive( const std::string &pad_name, sim::Clock::time_point time ) {
  auto instance = m_instance.lock();
  auto pad = m_instance.lock()->pad( pad_name );
  if ( !pad ) {
    return {{}, "no pad: " + pad_name};
  }
  
  if ( pad->available() > 0 ) {
    return Pad::Private::pull( pad );
  }
  
  // TODO lock state
  if ( m_state != State::RUN ) {
    return { {}, "already waiting" }; // Activity already waiting
  }
  m_state = State::PAUSE;
  auto future = Simulation::Private::activityPadReceive(
      instance->owner(),
      m_activity.shared_from_this(),
      pad_name,
      time );
  auto rv = future.get();
  if (!rv) {
    return { {}, "receive canceled" };
  }
  return Pad::Private::pull( pad );
}
#endif // ACPP_LESSON > 4

#if ACPP_LESSON > 3
bool Activity::Impl::padSend( const std::string &pad_name, const std::any &payload, const std::string &activity_name ) {
  auto pad = m_instance.lock()->pad( pad_name );
  if ( !(pad && pad->peer()) ) {
    return false;
  }
  return Pad::Private::push( pad->peer(), payload );
}
#endif // ACPP_LESSON > 3

#if ACPP_LESSON > 4
bool Activity::Impl::padSend( const std::string &pad_name, const std::any &payload ) {
  auto pad = m_instance.lock()->pad( pad_name );
  if ( !(pad && pad->peer()) ) {
    return false;
  }
  return Pad::Private::push( pad->peer(), payload );
}
#endif // ACPP_LESSON > 4

#if ACPP_LESSON > 3
acpp::value_result<std::any> Activity::padReceive( const std::string &pad_name, const std::string &activity_name ) {
  return impl->padReceive( pad_name, {}, activity_name );
}
acpp::value_result<std::any> Activity::padReceive( const std::string &pad_name, sim::Clock::duration timeout, const std::string &activity_name ) {
  return impl->padReceive( pad_name, owner()->owner()->simtime() + timeout, activity_name );
}
bool Activity::padSend( const std::string &pad_name, const std::any &payload, const std::string &activity_name ) {
  return impl->padSend( pad_name, payload, activity_name );
}
#endif // ACPP_LESSON > 3

#if ACPP_LESSON > 4
acpp::value_result<std::any> Activity::padReceive( const std::string &pad_name ) {
  return impl->padReceive( pad_name, {} );
}
acpp::value_result<std::any> Activity::padReceive( const std::string &pad_name, sim::Clock::duration timeout ) {
  return impl->padReceive( pad_name, owner()->owner()->simtime() + timeout );
}
bool Activity::padSend( const std::string &pad_name, const std::any &payload ) {
  return impl->padSend( pad_name, payload );
}
#endif // ACPP_LESSON > 4


struct Pad::Impl {
  Impl(
      Pad &pad,
      std::shared_ptr<Instance> instance,
      const PadSpec &spec,
      const std::string &name ) :
      m_pad{ pad },
      m_instance{ instance },
      m_spec{ spec },
      m_name{ name } {
    if ( name.empty() ) {
      throw "name not supplied";
    }
  }

  bool connect( std::shared_ptr<Instance> instance, const std::string &pad_name );
  acpp::value_result<std::any> pull();
  bool push( const std::any &payload );
  bool push( std::any &&payload );

  Pad &m_pad; // Pad owns Pad::Impl
  std::weak_ptr<Instance> m_instance;
  PadSpec m_spec;
  std::string m_name;
  std::shared_ptr<Pad> m_peer;
  std::shared_mutex m_queue_mut;
  std::deque<std::any> m_queue;
};

Pad::Pad( std::shared_ptr<Instance> instance, const PadSpec &spec, const std::string &name ) :
    impl( new Impl{ *this, instance, spec, name } ) {
}

Pad::~Pad() noexcept = default;
Pad::Pad( Pad &&other ) = default;
Pad &Pad::operator=( Pad &&other ) = default;

PadSpec Pad::spec() const {
  return impl->m_spec;
}

std::string Pad::name() const {
  return impl->m_name;
}

std::shared_ptr<Instance> Pad::owner() const {
  return impl->m_instance.lock();
}

std::shared_ptr<Pad> Pad::peer() const {
  return impl->m_peer;
}

bool Pad::connect( std::shared_ptr<Instance> instance, const std::string &pad_name ) {
  return impl->connect( instance, pad_name );
}

bool Pad::Impl::connect( std::shared_ptr<Instance> instance, const std::string &pad_name ) {
  if (!instance) {
    return false;
  }
  auto peer = instance->pad( pad_name );
  if (!peer || peer->impl.get() == this) {
    return false;
  }

  if (m_peer == peer) {
    return true;
  }

  // TODO add new peer disconnect if needed
  if (peer->peer()) {
    return false;
  }

  // disconnect if needed
  if (m_peer && m_peer->impl->m_peer && m_peer->impl->m_peer->impl.get() == this) {
    m_peer->impl->m_peer.reset();
  }

  // now connect
  m_peer = peer;
  peer->impl->m_peer = m_pad.shared_from_this();

  return true;
}

size_t Pad::available() const {
  std::shared_lock lock { impl->m_queue_mut };
  return impl->m_queue.size();
}

acpp::value_result<std::any> Pad::Private::pull( std::shared_ptr<Pad> pad ) {
  return pad->impl->pull();
}

acpp::value_result<std::any> Pad::Impl::pull() {
  std::unique_lock lock { m_queue_mut };
  if (m_queue.empty()) {
    return { {}, "nothing waiting" };
  }
  auto msg = m_queue.front();
  m_queue.pop_front();

  return acpp::value_result<std::any>( msg );
}

bool Pad::Private::push( std::shared_ptr<Pad> pad, const std::any &payload ) {
  return pad->impl->push( payload );
}

bool Pad::Impl::push( const std::any &payload ) {
  std::unique_lock lock { m_queue_mut };
  m_queue.push_back( payload );
  return true;
}

}  // namespace sim
