// CxxSimulator.cpp : Defines the entry point for the application.
//

#include <CxxSimulator/Simulator.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace sim {

class Simulation::Impl {

};

class Simulator::Impl {
  
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
}

}  // namespace sim
