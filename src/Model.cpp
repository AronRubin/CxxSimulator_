// CxxSimulator.cpp : Defines the entry point for the application.
//

#include <CxxSimulator/Simulator.h>
#include <CxxSimulator/Model.h>
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
  std::unordered_map<std::string, acpp::unstructured_value> m_parameters;
  std::unordered_map<std::string, std::shared_ptr<Activity>> m_activities;
};

Instance::~Instance() = default;
Instance::Instance( Instance &&other ) noexcept = default;
Instance &Instance::operator=( Instance &&other ) noexcept = default;

Instance::Instance( Simulation &sim, std::shared_ptr<Model> model, const std::string &name ) : impl( new Impl ) {
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

struct Activity::Impl {
  ActivitySpec m_spec;
  State m_state;
  State m_pending_state;
};

Activity::Activity( Simulation &sim, const ActivitySpec &spec ) : impl( new Impl ) {
  impl->m_spec = spec;
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
  
Activity::State Activity::state( State &pending ) const {
  return impl->m_pending_state;
}

}  // namespace sim
