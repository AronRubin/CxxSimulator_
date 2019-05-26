/**
 * Simulator.h
 */

#pragma once

#include <string>
#include <system_error>
#include <optional>
#include "cpp_utils.h"
#include "Model.h"

namespace sim {

// forward decl
class Model;

/**
 *
 */
class Simulator {
private:
  Simulator(); // mark as not publicly constructable

public:
  static Simulator &getInstance();

  // design choice: no throw
  /// <summary>
  /// Load a topology from a JSON description
  /// </summary>
  /// <param name="topo_json">The topology as a JSON string</param>
  /// <returns>success or failure as a VResult</returns>
  acpp::VResult<> loadTopology( const std::string &topo_json );

  // design choice: throw
  void addModel( const Model &model );
  void takeModel( Model &&model );

  // mark as immoveable
  Simulator( Simulator && ) = delete;
  Simulator &operator =(Simulator && ) = delete;

private:
  // PIMPL
  class Impl;
  std::unique_ptr<Impl> impl;
};

} // namespace sim

