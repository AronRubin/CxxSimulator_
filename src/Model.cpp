// CxxSimulator.cpp : Defines the entry point for the application.
//

#include <CxxSimulator/Simulator.h>
#include <CxxSimulator/Model.h>
#include "Simulation_p.h"

#include <nlohmann/json.hpp>

#include <map>
#include <string>
#include <memory>

using json = nlohmann::json;

namespace sim {

struct Model::Impl {
  std::string m_name;
  std::unordered_map<std::string, acpp::unstructured_value> m_parameters;
  std::unordered_map<std::string, ActivitySpec> m_activity_specs;
  std::unordered_map<std::string, PadSpec> m_pad_specs;
};

Model::Model( const std::string &name ) : impl( new Impl ) {
  impl->m_name = name;
}

Model::~Model() = default;

void Model::addPadSpec( const PadSpec &spec ) {
  if (spec.name.empty()) {

  }
  impl->m_pad_specs[spec.name] = spec;
}

std::string Model::name() const {
  return impl->m_name;
}

struct Instance::Impl {
  std::string m_name;
  std::shared_ptr<Model> m_model;
  PropertyList m_parameters;
  std::unordered_map<std::string, std::shared_ptr<Activity>> m_activities;

  std::shared_ptr<Simulation> m_simulation;
};

Instance::~Instance() = default;
Instance::Instance( Instance &&other ) noexcept = default;
Instance &Instance::operator=( Instance &&other ) noexcept = default;

Instance::Instance(
    Simulation &sim,
    std::shared_ptr<Model> model,
    const std::string &name ) : impl( new Impl ) {
  if (!model) {
    throw "model not supplied";
  }
  if (name.empty()) {
    throw "name not supplied";
  }
  impl->m_name = name;
  impl->m_model = model;
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

acpp::unstructured_value Instance::parameter( const std::string &name ) const {
  auto iter = impl->m_parameters.find( name );
  if ( iter == impl->m_parameters.end() ) {
    return {};
  }
  return iter->second;
}

struct Activity::Impl {
  std::weak_ptr<Activity> m_activity;
  std::weak_ptr<Instance> m_instance;
  ActivitySpec m_spec;
  State m_state = State::INIT;
  std::thread m_worker;
  
  std::mutex m_state_mut;
  std::condition_variable m_state_cnd;

  void waitUntil( sim::Clock::time_point time );
  acpp::void_result<> waitOn( const std::string &name );
  acpp::void_result<> waitOn( const std::string &name, sim::Clock::duration timeout );

  void workerFunc();
};

Activity::Activity( std::shared_ptr<Instance> instance, const ActivitySpec &spec ) : impl( new Impl ) {
  impl->m_activity = weak_from_this();
  impl->m_instance = instance;
  impl->m_spec = spec;
  impl->m_worker = std::thread( &Activity::Impl::workerFunc, impl.get() );
}
Activity::~Activity() = default;
Activity::Activity( Activity &&other ) noexcept = default;
Activity &Activity::operator=( Activity &&other ) noexcept = default;

ActivitySpec Activity::spec() const {
  return impl->m_spec;
}

Activity::State Activity::state() const {
  return impl->m_state;
}

std::shared_ptr<Instance> Activity::owner() const {
  return impl->m_instance.lock();
}

std::string Activity::name() const {
  return impl->m_spec.name;
}

void Activity::Impl::workerFunc() {
  if ( !m_spec.function ) {
    return;
  }
  std::unique_lock<std::mutex> state_lock( m_state_mut );
  m_state_cnd.wait( state_lock, [this]() { return m_state != State::INIT && m_state != State::PAUSE; } );
  if ( m_state == State::RUN ) {
    std::invoke( m_spec.function, *m_instance.lock(), *m_activity.lock() );
  }
}

void Activity::Impl::waitUntil( sim::Clock::time_point time ) {
  // TODO lock
  if (m_state != State::RUN) {
    return;
  }
  m_state = State::PAUSE;
  auto instance = m_instance.lock();
  auto activity = m_activity.lock();
  auto future = Simulation::Private::insertResumeActivity(
      instance->owner(),
      activity,
      time );
  future.get();
}

void Activity::waitUntil( sim::Clock::time_point time ) {
  impl->waitUntil( time );
}

void Activity::waitFor( sim::Clock::duration dur ) {
  auto instance = impl->m_instance.lock();
  waitUntil( instance->owner()->simtime() + dur );
}

acpp::void_result<> Activity::Impl::waitOn( const std::string &signal_name ) {
  // TODO lock
  if (m_state != State::RUN) {
    return {{}, "Activity already waiting"};
  }
  m_state = State::PAUSE;
  auto instance = m_instance.lock();
  auto activity = m_activity.lock();
  auto future = Simulation::Private::activityReceive( instance->owner(), activity, signal_name );
  return future.get();
}

acpp::void_result<> Activity::Impl::waitOn( const std::string &signal_name, sim::Clock::duration timeout ) {
  // TODO lock
  if (m_state != State::RUN) {
    return {{}, "Activity already waiting"};
  }
  m_state = State::PAUSE;
  auto instance = m_instance.lock();
  auto activity = m_activity.lock();
  auto future = Simulation::Private::activityReceive(
      instance->owner(),
      activity,
      signal_name,
      instance->owner()->simtime() + timeout );
  return future.get();
}

acpp::void_result<> Activity::waitOn( const std::string &signal_name ) {
  return impl->waitOn( signal_name );
}

acpp::void_result<> Activity::waitOn( const std::string &signal_name, sim::Clock::duration timeout ) {
  return impl->waitOn( signal_name, timeout );
}


}  // namespace sim
