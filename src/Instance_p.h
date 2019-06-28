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

}  // namespace sim

#endif  // INSTANCE_P_H_INCLUDED
