// CxxSimulator.cpp : Defines the entry point for the application.
//

#include <CxxSimulator/Simulator.h>
#include <nlohmann/json.hpp>

#include <map>
#include <vector>
#include <string>
#include <chrono>
#include <memory>
#include <functional>
#include <algorithm>
#include <deque>
#include <list>
#include <any>

using json = nlohmann::json;

namespace sim {

struct SimEvent {
  enum class Type {
    STATE_CHANGE,
    SPAWN_INSTANCE,
    SPAWN_ACTIVITY,
    RESUME_ACTIVITY,
    PAD_SEND
  };

  SimEvent(
      Type type,
      const Clock::time_point &time,
      const std::string &name,
      const std::string &owner = {},
      const PropertyList &parameters = {},
      std::shared_ptr<Activity> activity = {} ) :
      type( type ),
      time( time ),
      name( name ),
      owner( owner ),
      parameters( parameters ),
      activity( activity ) {}
  ~SimEvent() = default;
  SimEvent( SimEvent & ) = default;
  SimEvent &operator=( SimEvent & ) = default;
  SimEvent( SimEvent && ) = default;
  SimEvent &operator=( SimEvent && ) = default;

  Type type;
  Clock::time_point time;
  std::string name;
  std::string owner;
  PropertyList parameters;
  std::shared_ptr<Activity> activity;

  friend bool operator<( const SimEvent &eva, const SimEvent & evb ) {
    return eva.time < evb.time;
  }
};

struct Simulation::Impl {
  Clock::time_point m_simtime;
  State m_state = State::INIT;
  State m_pending_state = State::INIT;

  PropertyList m_parameters;
  
  std::map<std::string, Instance> m_instances;

  std::deque<SimEvent> m_events;

  acpp::void_result<> insertSpawnInstance(
    const std::string &name,
    const std::string &model,
    const PropertyList &parameters,
    const Clock::time_point &time );

  acpp::void_result<> insertSpawnActivity(
    const std::string &name,
    const std::string &instance,
    const PropertyList &parameters,
    const Clock::time_point &time );
};

Simulation::Simulation() : impl( new Impl ) {}

Simulation::~Simulation() = default;

acpp::void_result<> Simulation::spawnInstance(
    const std::string &name,
    const std::string &model,
    const PropertyList &parameters,
    const Clock::time_point &time ) {
  return impl->insertSpawnInstance( name, model, parameters, time );
}

acpp::void_result<> Simulation::Impl::insertSpawnInstance(
    const std::string &name,
    const std::string &model,
    const PropertyList &parameters,
    const Clock::time_point &time ) {
  auto iiter = m_instances.find( name );
  if (iiter != m_instances.end()) {
    return { {}, "instance not unique" };
  }
  // TODO lock here
  // check if this is pending spawn
  auto eiter = std::find_if(
      m_events.begin(),
      m_events.end(),
      [name]( const SimEvent &event ) {
        return event.type == SimEvent::Type::SPAWN_INSTANCE && event.name == name;
      } );
  if (eiter != m_events.end()) {
    return { {}, "instance not unique" };
  }
  
  Clock::time_point event_time = time;
  if (time.time_since_epoch() == Clock::duration::zero()) {
    event_time = m_simtime;
  }
  m_events.emplace_back( SimEvent::Type::SPAWN_INSTANCE, event_time, name, model, parameters );
  std::push_heap( m_events.begin(), m_events.end() );

  return {};
}

acpp::void_result<> Simulation::Impl::insertSpawnActivity(
    const std::string &name,
    const std::string &instance,
    const PropertyList &parameters,
    const Clock::time_point &time ) {
  auto iiter = m_instances.find( instance );
  if (iiter == m_instances.end()) {
    // TODO allow if there is a spawn instance of the right name before time
    return { {}, "instance not found" };
  }
  // TODO lock here
  Clock::time_point event_time = time;
  if (time.time_since_epoch() == Clock::duration::zero()) {
    event_time = m_simtime;
  }
  m_events.emplace_back( SimEvent::Type::SPAWN_ACTIVITY, event_time, name, instance, parameters );
  std::push_heap( m_events.begin(), m_events.end() );

  return {};
}

// global parameters
acpp::void_result<> Simulation::setParameter( const std::string &name, const acpp::unstructured_value &value ) {
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
  impl->m_pending_state = state;
  return {};
}

struct Simulator::Impl {
  std::unordered_map<std::string, std::shared_ptr<Model>> m_models;
  std::vector<Simulation> m_simulations;
};

Simulator::Simulator() : impl( new Impl ) {
}

void Simulator::reset() {
  impl = std::make_unique<Impl>();
}

Simulator &Simulator::getInstance() {
  static Simulator instance;
  return instance;
}

acpp::value_result<Simulation> Simulator::loadTopology( const std::string &topo_json ) {
  return { {}, "unimplemented" };
}

void Simulator::addModel( std::shared_ptr<Model> model ) {
  if (!model) {
    return;
  }
  impl->m_models.emplace( model->name(), model );
}

std::shared_ptr<Model> Simulator::model( const std::string &name ) {
  auto iter = impl->m_models.find( name );
  if (iter != impl->m_models.end()) {
    return iter->second;
  }
  return nullptr;
}

}  // namespace sim
