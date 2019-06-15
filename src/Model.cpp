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
};

Model::Model() : impl( new Impl ) {
}

void Model::addPadSpec( const PadSpec &spec ) {

}


class Instance::Impl {
public:
  std::string m_name;
  std::shared_ptr<Model> m_model;
  std::unordered_map<std::string, acpp::unstructured_value> m_parameters;
  std::unordered_map<std::string, std::shared_ptr<Activity>> m_activities;
};

Instance::Instance() : impl( new Impl ) {
}

class Activity::Impl {
public:
};

Activity::Activity() : impl( new Impl ) {
}

}  // namespace sim
