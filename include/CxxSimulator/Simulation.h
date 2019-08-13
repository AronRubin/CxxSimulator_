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
  Simulation( Simulation &&other );
  Simulation &operator=( Simulation &&other );
  ~Simulation() noexcept;

  /**
   * @brief Get a reference to an instance that has already spawned
   * @param name the name of the instance to return
   * @return std::shared_ptr<Instance>
   */
  std::shared_ptr<Instance> instance( const std::string &name ) const;
  /**
   * @brief Get a list of references to all spawned activities
   * @return std::vector<std::shared_ptr<Instance>> 
   */
  std::vector<std::shared_ptr<Instance>> instances() const;

  /**
   * @brief Get a variant of a simulation-global parameter
   * @param name the name of the parameter to get
   * @return acpp::unstructured_value 
   */
  acpp::unstructured_value parameter( const std::string &name ) const;
  /**
   * @brief Get a simulation-global parameter as a specific type, converting if necessary
   * @tparam T the type of parameter to return
   * @param name the name of the parameter to get
   * @return std::optional<T> 
   */
  template <typename T>
  std::optional<T> parameter( const std::string &name ) const {
    return acpp::get_as<T>( parameter( name ) );
  }
  /**
   * @brief Set a simulation-global parameter, replacing any previous value
   * @param name the name of the parameter to set
   * @param value the new value of the parameter
   * @return acpp::void_result<> A success or error indicator
   */
  acpp::void_result<> setParameter( const std::string &name, const acpp::unstructured_value &value );
  /**
   * @brief Get the current simulation time
   * @return Clock::time_point the current simulation time
   */
  Clock::time_point simtime() const;
  /**
   * @brief Get the current simulation state
   * @return State the current simulation state
   */
  State state() const;
  /**
   * @brief Get the current simulation state and a pending state if the simulation is transitioning
   * @param pending [out] the value of the last requested state
   * @return State the value of the current simulation state
   */
  State state( State &pending ) const;
  /**
   * @brief Request the simulation to transition to a new state
   * @param state the state to transition to
   * @return acpp::void_result<> A success or error indicator
   */
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
