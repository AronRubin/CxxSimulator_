/**
 * Simulator.h
 */

#ifndef SIMULATOR_H_INCLUDED
#define SIMULATOR_H_INCLUDED

#include "cpp_utils.h"
#include "Model.h"
#include "Simulation.h"
#include "Common.h"

#include <memory>
#include <functional>
#include <string>
#include <system_error>
#include <optional>
#include <iterator>

namespace sim {

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
   * @brief Load a topology from a JSON description
   * @param topo_json The topology as a JSON string
   * @return the created simulation or failure as a value_result
   */
  std::shared_ptr<Simulation> loadTopology( const std::string &topo_json );

  // design choice: throw
  
  /**
   * @brief Register a model for instancing
   * @param model a model
   */
  void addModel( std::shared_ptr<Model> model );

  /**
   * @brief Register a model for instancing
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
