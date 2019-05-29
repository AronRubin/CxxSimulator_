// CxxSimulator.cpp : Defines the entry point for the application.
//

#include <CxxSimulator/Simulator.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace sim {

class Simulation::Impl {};

class Simulator::Impl {};

Simulator::Simulator() : impl( new Impl ) {
}

Simulator &Simulator::getInstance() {
  static Simulator instance;
  return instance;
}

acpp::value_result<Simulation> Simulator::loadTopology( const std::string &topo_json ) {
  return { {}, "unimplemented" };
}

void Simulator::addModel( const Model &model ) {
}

void Simulator::takeModel( Model &&model ) {
}

}  // namespace sim
