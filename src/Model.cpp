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

class Model::Impl {
public:
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


class Instance::Impl {
public:
  std::string m_name;
  std::shared_ptr<Model> m_model;
  std::unordered_map<std::string, acpp::unstructured_value> m_parameters;
  std::unordered_map<std::string, std::shared_ptr<Activity>> m_activities;
};

Instance::Instance( Simulator &sim, std::shared_ptr<Model> model, const std::string &name ) : impl( new Impl ) {
  if (!model) {
    throw "model not supplied";
  }
  if (name.empty()) {
    throw "name not supplied";
  }
  impl->m_name = name;
  impl->m_model = model;
}

class Activity::Impl {
public:
};

Activity::Activity() : impl( new Impl ) {
}

}  // namespace sim
