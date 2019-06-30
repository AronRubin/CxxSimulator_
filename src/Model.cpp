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
  Impl( const std::string &name ) : m_name{ name } {
    if (name.empty()) {
      throw "name not specified";
    }
  }
  ~Impl() = default;
  Impl( Impl &&other ) noexcept = default;
  Impl &operator=( Impl &&other ) noexcept = default;
  
  std::string m_name;
  PropertyList m_parameters;
  std::unordered_map<std::string, ActivitySpec> m_activity_specs;
  std::unordered_map<std::string, PadSpec> m_pad_specs;
};

Model::Model( const std::string &name ) : impl( new Impl{ name } ) {
}

Model::~Model() = default;

void Model::addPadSpec( const PadSpec &spec ) {
  if (spec.name.empty()) {
    return;
  }
  impl->m_pad_specs[spec.name] = spec;
}

void Model::addActivitySpec( const ActivitySpec &spec ) {
  if (spec.name.empty()) {
    return;
  }
  impl->m_activity_specs[spec.name] = spec;
}

std::string Model::name() const {
  return impl->m_name;
}

std::vector<PadSpec> Model::pads() {
  std::vector<PadSpec> pads;
  for ( const auto &padent : impl->m_pad_specs ) {
    pads.push_back( padent.second );
  }
  return pads;
}

PadSpec Model::pad( const std::string &name ) {
  auto iter = impl->m_pad_specs.find( name );
  if ( iter == impl->m_pad_specs.end() ) {
    return {};
  }
  return iter->second;
}

std::vector<ActivitySpec> Model::activities() {
  std::vector<ActivitySpec> activities;
  for ( const auto &activityent : impl->m_activity_specs ) {
    activities.push_back( activityent.second );
  }
  return activities;
}

ActivitySpec Model::activity( const std::string &name ) {
  auto iter = impl->m_activity_specs.find( name );
  if ( iter == impl->m_activity_specs.end() ) {
    return {};
  }
  return iter->second;
}

}  // namespace sim
