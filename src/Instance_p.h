/**
 * Instance_p.h
 * Instance private interface with simulation
 */

#ifndef SIM_INSTANCE_P_H_INCLUDED
#define SIM_INSTANCE_P_H_INCLUDED

#include <CxxSimulator/Simulator.h>
#include <CxxSimulator/Instance.h>

#include <memory>
#include <string>

namespace sim {

struct Instance::Private {
  static bool insertActivity(
      std::shared_ptr<Instance> instance,
      const std::string &spec,
      const std::string &name );
};

struct Pad::Private {
  static acpp::value_result<std::any> pull( std::shared_ptr<Pad> pad );
  static bool push( std::shared_ptr<Pad> pad, const std::any &payload );
};

}  // namespace sim

#endif  // INSTANCE_P_H_INCLUDED
