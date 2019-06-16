// CxxSimulator.cpp : Defines the entry point for the application.
//

#include <CxxSimulator/Simulator.h>
#include <nlohmann/json.hpp>

#include <map>
#include <vector>
#include <string>

using json = nlohmann::json;

namespace sim {

struct Simulation::Impl {

};

struct Simulator::Impl {
  std::unordered_map<std::string, std::shared_ptr<Model>> m_models;
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
  if( !model ) {
    return;
  }
  impl->m_models.emplace( model->name(), model );
}

}  // namespace sim
