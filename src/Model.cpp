// CxxSimulator.cpp : Defines the entry point for the application.
//

#include <CxxSimulator/Simulator.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace sim {

class Model::Impl {};

Model::Model() : impl( new Impl ) {}

void addPad( const PadSpec &spec );

}  // namespace sim
