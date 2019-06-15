/**
 * Simulator.h
 */

#ifndef SIMULATOR_H_INCLUDED
#define SIMULATOR_H_INCLUDED

#include <string>
#include <system_error>
#include <optional>
#include <iterator>
#include "cpp_utils.h"
#include "Model.h"

namespace sim {

// forward decls
class Model;
class Instance;
class Simulator;

class Simulation {
public:
  using InstanceIterator = std::vector<Instance>::iterator;
  using ConstInstanceIterator = std::vector<Instance>::const_iterator;

  Simulation() = default;
  ~Simulation() = default;

  // sure you can _copy_, but _should_ you?
  Simulation( const Simulation &other );
  Simulation( Simulation &&other );
  
  acpp::value_result<std::reference_wrapper<Instance>> emplace( Instance &&instance );
  acpp::value_result<std::reference_wrapper<Instance>> emplace( const Model &stats, const std::string &name );
  std::optional<std::reference_wrapper<Instance>> getInstance( const std::string &name );

  // global parameters
  acpp::void_result<> setParameter( const std::string &name, const std::string &value );
  std::optional<std::string> getParameter( const std::string &name );

private:
  friend class Simulator;

  // PIMPL
  class Impl;
  std::unique_ptr<Impl> impl;
};

/**
 * The manager and factory of simulations
 * This is a singleton
 */
class Simulator {
private:
  Simulator(); // mark as not publicly constructable

public:
  static Simulator &getInstance();

  void reset();

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
