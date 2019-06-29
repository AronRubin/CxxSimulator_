/**
 * Simulation_p.h
 * Simulation private interface with instances
 */

#ifndef SIMULATION_P_H_INCLUDED
#define SIMULATION_P_H_INCLUDED

#include <CxxSimulator/Simulator.h>
#include <CxxSimulator/Instance.h>
#include <CxxSimulator/Model.h>

#include <memory>
#include <functional>
#include <string>
#include <any>
#include <future>

namespace sim {

struct Simulation::Private {
  static std::future<bool> insertResumeActivity(
      std::shared_ptr<Simulation> simulation,
      std::shared_ptr<Activity> activity,
      const Clock::time_point &time );
  static std::future<bool> activityWaitOn(
      std::shared_ptr<Simulation> simulation,
      std::shared_ptr<Activity> activity,
      const std::string &signal_name,
      const Clock::time_point &time = {} );
};

} // namespace sim

#endif // SIMULATION_P_H_INCLUDED
