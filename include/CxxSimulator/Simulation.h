/**
 * Simulation.h
 */

#ifndef SIMULATION_H_INCLUDED
#define SIMULATION_H_INCLUDED

#include "cpp_utils.h"
#include "Model.h"
#include "Instance.h"
#include "Clock.h"
#include "Common.h"

#include <memory>
#include <functional>
#include <string>
#include <system_error>
#include <optional>
#include <vector>

namespace sim {

// forward decls
class Simulator;

class Simulation : public std::enable_shared_from_this<Simulation> {
public:
  enum class State { INIT, RUN, PAUSE, DONE };

  Simulation();
  ~Simulation();
  Simulation( Simulation &&other );
  Simulation &operator=( Simulation &&other );

  /**
   * @brief Get a reference to an instance that has already spawned
   */
  std::shared_ptr<Instance> instance( const std::string &name ) const;
  std::vector<std::shared_ptr<Instance>> instances() const;

  acpp::unstructured_value parameter( const std::string &name ) const;
  template <typename T>
  std::optional<T> parameter( const std::string &name ) const {
    return acpp::get_as<T>( parameter( name ) );
  }
  acpp::void_result<> setParameter( const std::string &name, const acpp::unstructured_value &value );
  Clock::time_point simtime() const;
  State state() const;
  State state( State &pending ) const;
  acpp::void_result<> setState( const State &state );
  
  /**
   * @brief Request an instance to be spawned in the simulation
   * @param name name of this instance (must be unique within the simulation)
   * @param model name of the model used (must be registered in the simulator)
   * @param parameters additional parameters to forward to the starting
   * @return success or error
   */
  acpp::void_result<> spawnInstance(
      const std::string &model,
      const std::string &name,
      const PropertyList &parameters = {},
      const Clock::time_point &time = {} );

  /**
   * @brief Request an activity to be spawned for an instance in the simulation
   * Note the start activity is automatically spawned with its instance.
   * @param spec name of the specification for this activity (within the model)
   * @param name name of this activity
   * @param instance name of the instance (must have spawned before time)
   * @param time simulation time to spawn the activity or {} for immediate
   * @return Returned value description
   */
  acpp::void_result<> spawnActivity(
      const std::string &spec,
      const std::string &name,
      const std::string &instance,
      const Clock::time_point &time = {} );

private:
  friend class Simulator;

  // PIMPL
  class Impl;
  std::unique_ptr<Impl> impl;

public:
  /**
   * @brief Oxymoron advertising privates for those in the know
   */
  class Private;
};


}  // namespace sim

#endif  // SIMULATION_H_INCLUDED
