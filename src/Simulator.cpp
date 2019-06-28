// CxxSimulator.cpp : Defines the entry point for the application.
//

#include <CxxSimulator/Simulator.h>
#include "Simulation_p.h"

#include <nlohmann/json.hpp>

#include <map>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>

using json = nlohmann::json;

namespace sim {

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

std::shared_ptr<Simulation> Simulator::loadTopology( const std::string &topo_json ) {
  return {};  // unimplemented
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
