// CxxSimulator.cpp : Defines the entry point for the application.
//

#include <CxxSimulator/Simulator.h>
#include <CxxSimulator/Model.h>
#include "Simulation_p.h"
#include "Instance_p.h"

#include <map>
#include <string>
#include <memory>

namespace sim {

struct Instance::Impl {
  Impl(
      Instance &instance,
      std::shared_ptr<Simulation> sim,
      std::shared_ptr<Model> model,
      const std::string &name,
      const PropertyList &parameters ) :
      m_instance{ instance },
      m_simulation{ sim },
      m_model{ model },
      m_name{ name },
      m_parameters{ parameters } {
    if ( name.empty() ) {
      throw "name not supplied";
    }
  }
  
  ~Impl() = default;
  Impl( Impl &&other ) noexcept = default;
  Impl &operator=( Impl &&other ) noexcept = default;

  bool insertActivity( const std::string &spec, const std::string &name );

  Instance &m_instance; // Instance owns Instance::Impl
  std::shared_ptr<Simulation> m_simulation;
  std::shared_ptr<Model> m_model;
  std::string m_name;
  PropertyList m_parameters;
  std::unordered_map<std::string, std::shared_ptr<Activity>> m_activities;
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

bool Instance::Impl::insertActivity( const std::string &spec_name, const std::string &name ) {
  auto spec = m_model->activity( spec_name );
  if ( spec.name.empty() ) {
    return false;
  }
  auto aiter = m_activities.find( name );
  if ( aiter != m_activities.end() ) {
    return false;
  }
  auto rv = m_activities.emplace( name, std::make_shared<Activity>( m_instance.shared_from_this(), spec, name ) );
  return rv.second;
}

bool Instance::Private::insertActivity(
    std::shared_ptr<Instance> instance,
    const std::string &spec,
    const std::string &name ) {
  if ( !instance ) {
    return false;
  }
  return instance->impl->insertActivity( spec, name );
}

struct Activity::Impl {
  Impl(
      Activity &activity,
      std::shared_ptr<Instance> instance,
      const ActivitySpec &spec,
      const std::string &name ) :
      m_activity{ activity },
      m_instance{ instance },
      m_spec{ spec },
      m_name{ name } {
    if ( name.empty() ) {
      throw "name not supplied";
    }
  }
  Activity &m_activity; // Activity owns Activity::Impl
  std::weak_ptr<Instance> m_instance;
  ActivitySpec m_spec;
  std::string m_name;
  State m_state = State::INIT;
  std::thread m_worker;
  std::mutex m_state_mut;
  std::condition_variable m_state_cnd;

  void waitUntil( const sim::Clock::time_point &time );
  bool waitOn( const std::string &name );
  bool waitOn( const std::string &name, sim::Clock::duration timeout );

  void workerFunc();
};

Activity::Activity( std::shared_ptr<Instance> instance, const ActivitySpec &spec, const std::string &name ) :
    impl( new Impl{ *this, instance, spec, name } ) {
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
    state_lock.unlock();
    std::invoke( m_spec.function, *m_instance.lock(), m_activity );
  }
}

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

void Activity::waitUntil( const sim::Clock::time_point &time ) {
  impl->waitUntil( time );
}

void Activity::waitFor( sim::Clock::duration dur ) {
  auto instance = impl->m_instance.lock();
  waitUntil( instance->owner()->simtime() + dur );
}

bool Activity::Impl::waitOn( const std::string &signal_name ) {
  // TODO lock
  if ( m_state != State::RUN ) {
    return false; // Activity already waiting
  }
  m_state = State::PAUSE;
  auto future = Simulation::Private::activityWaitOn(
      m_instance.lock()->owner(),
      m_activity.shared_from_this(),
      signal_name );
  return future.get();
}

bool Activity::Impl::waitOn( const std::string &signal_name, sim::Clock::duration timeout ) {
  // TODO lock
  if ( m_state != State::RUN ) {
    return false; // Activity already waiting
  }
  m_state = State::PAUSE;
  auto instance = m_instance.lock();
  auto future = Simulation::Private::activityWaitOn(
      m_instance.lock()->owner(),
      m_activity.shared_from_this(),
      signal_name,
      instance->owner()->simtime() + timeout );
  return future.get();
}

bool Activity::waitOn( const std::string &signal_name ) {
  return impl->waitOn( signal_name );
}

bool Activity::waitOn( const std::string &signal_name, sim::Clock::duration timeout ) {
  return impl->waitOn( signal_name, timeout );
}

}  // namespace sim
