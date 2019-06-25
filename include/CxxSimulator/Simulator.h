/**
 * Simulator.h
 */

#ifndef SIMULATOR_H_INCLUDED
#define SIMULATOR_H_INCLUDED

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
#include <iterator>

namespace sim {

// forward decls
class Model;
class Instance;
class Simulator;

class Simulation {
public:
  using InstanceIterator = std::vector<Instance>::iterator;
  using ConstInstanceIterator = std::vector<Instance>::const_iterator;
  enum class State { INIT, RUN, PAUSE, DONE };

  Simulation();
  ~Simulation();

  // sure you can _copy_, but _should_ you?
  Simulation( const Simulation &other );
  Simulation( Simulation &&other );
  
  
  acpp::void_result<> spawnInstance(
    const std::string &name,
    const std::string &model,
    const PropertyList &parameters = {},
    const Clock::time_point &time = {} );
  
  acpp::void_result<> spawnActivity(
    const std::string &name,
    const std::string &instance,
    const PropertyList &parameters = {},
    const Clock::time_point &time = {} );
  
  std::shared_ptr<Instance> instance( const std::string &name ) const;

  // global parameters
  acpp::unstructured_value parameter( const std::string &name ) const;
  template <typename T>
  std::optional<T> parameter( const std::string &name ) const {
    return acpp::get_as<T>( parameter( name ) );
  }

  acpp::void_result<> setParameter(
    const std::string &name,
    const acpp::unstructured_value &value );

  Clock::time_point simtime() const;

  State state() const;
  State state( State &pending ) const;

  acpp::void_result<> setState( const State &state );

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

/**
 * @brief The manager and factory of simulations
 * This is a singleton
 */
class Simulator {
private:
  Simulator(); // mark as not publicly constructable

public:
  static Simulator &getInstance();

  void reset();

  std::shared_ptr<Model> model( const std::string &name );

  // design choice: no throw
  
  /**
   * Load a topology from a JSON description
   * @param topo_json The topology as a JSON string
   * @return the created simulation or failure as a value_result
   */
  acpp::value_result<Simulation> loadTopology( const std::string &topo_json );

  // design choice: throw
  
  /**
   * Register a model for instancing
   * 
   * @param model a model
   */
  void addModel( std::shared_ptr<Model> model );

  /**
   * Register a model for instancing
   * 
   * @tparam ModelType a model type
   */
  template <typename ModelType>
  void addModel() {
    addModel( std::make_shared<ModelType>() );
  }

  // mark as immoveable
  Simulator( Simulator && ) = delete;
  Simulator &operator =(Simulator && ) = delete;

private:
  // PIMPL
  class Impl;
  std::unique_ptr<Impl> impl;
};

} // namespace sim

#endif // SIMULATOR_H_INCLUDED
