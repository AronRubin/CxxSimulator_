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
#include <queue>
#include <list>

using json = nlohmann::json;

namespace sim {

struct SimEvent {
  Clock::time_point time;
  std::shared_ptr<Activity> activity;

  friend bool operator<( const SimEvent &eva, const SimEvent & evb ) {
    return eva.time < evb.time;
  }
};

struct Simulation::Impl {
  Clock::time_point m_simtime;
  State m_state = State::INIT;
  State m_pending_state = State::INIT;

  std::unordered_map<std::string, acpp::unstructured_value> m_parameters;
  
  std::map<std::string, Instance> m_instances;

  std::priority_queue<SimEvent, std::list<SimEvent>> m_events;
};

Simulation::Simulation() : impl( new Impl ) {}

Simulation::~Simulation() = default;

acpp::value_result<std::reference_wrapper<Instance>> Simulation::emplace( Instance &&instance ) {
  auto [iter, success] = impl->m_instances.try_emplace( instance.name(), std::forward<Instance>( instance ) );
  if (success) {
    return acpp::value_result<std::reference_wrapper<Instance>>( std::ref( iter->second ) );
  } else {
    return { {}, "key not unique" };
  }
}

acpp::value_result<std::reference_wrapper<Instance>> Simulation::emplace( std::shared_ptr<Model> model, const std::string &name ) {
  auto [iter, success] = impl->m_instances.try_emplace( name, *this, model, name );
  if (success) {
    return acpp::value_result<std::reference_wrapper<Instance>>( std::ref( iter->second ) );
  } else {
    return { {}, "key not unique" };
  }
}

std::optional<std::reference_wrapper<Instance>> Simulation::instance( const std::string &name ) const {
  auto iter = impl->m_instances.find( name );
  if (iter == impl->m_instances.end()) {
    return {};
  }
  return { std::ref( iter->second ) };
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

Simulation::State Simulation::pendingState() const {
  return impl->m_pending_state;
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
