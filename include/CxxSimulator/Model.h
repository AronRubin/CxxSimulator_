/**
 * Model.h
 */

#ifndef MODEL_H_INCLUDED
#define MODEL_H_INCLUDED

#include "cpp_utils.h"
#include "Clock.h"
#include "Common.h"

#include <optional>
#include <string>
#include <system_error>
#include <bitset>
#include <map>
#include <any>

namespace sim {

class Simulator;
class Simulation;
class Model;
class Activity;
class Instance;

/**
 * Structure to specify how a pad is constructed (a connection point for a instance)
 */
struct PadSpec {
  enum class Flag : uint32_t { CAN_INPUT, CAN_OUTPUT, IS_TEMPLATE, BY_REQUEST, COUNT__ };

  PadSpec() = default; // results in an invalid/null padspec
  PadSpec( const std::string &name,
      const acpp::flagset<Flag> &flags = {Flag::CAN_INPUT, Flag::CAN_OUTPUT},
      const PropertyList &parameters = {} ) :
      name( name ),
      flags( flags ),
      parameters( parameters ) {}
  ~PadSpec() noexcept = default;
  PadSpec( const PadSpec &other ) = default;
  PadSpec( PadSpec &&other ) noexcept = default;
  PadSpec &operator=( const PadSpec &other ) = default;
  PadSpec &operator=( PadSpec &&other ) = default;

  std::string name;
  acpp::flagset<Flag> flags;
  PropertyList parameters;
};

/**
 * Structure to specify a how an activity is constructed
 */
struct ActivitySpec {
  using ActivityFunc = std::function<void( Instance &, Activity & )>;

  ActivitySpec() = default; // this will make an invalid/null spec
  ActivitySpec(
      const std::string &name,
      ActivityFunc func,
      const std::string &triggering_event = {} ) :
      name( name ),
      triggering_event( triggering_event ),
      function( func ) {}
  ~ActivitySpec() noexcept = default;
  ActivitySpec( const ActivitySpec &other ) = default;
  ActivitySpec( ActivitySpec &&other ) noexcept = default;
  ActivitySpec &operator=( const ActivitySpec &other ) = default;
  ActivitySpec &operator=( ActivitySpec &&other ) noexcept = default;

  std::string name;
  std::string triggering_event;
  ActivityFunc function;
};

/**
 * A model is an instance prototype and factory
 */
class Model : public std::enable_shared_from_this<Model> {
public:
  Model( const std::string &name );
  ~Model();
  Model( Model &&other ) = default;
  Model &operator=( Model &&other ) = default;

  std::string name() const;
  std::vector<PadSpec> pads();
  PadSpec pad( const std::string &name );
  std::vector<ActivitySpec> activities();
  ActivitySpec activity( const std::string &name );

  // entry point for the model
  virtual void startActivity( Instance &instance, Activity &activity ) = 0;
  
  // pad spec factory
  void addPadSpec( const PadSpec &spec );

  // activity spec factory
  void addActivitySpec( const ActivitySpec &spec );

private:
  class Impl;
  std::unique_ptr<Impl> impl;
};

}  // namespace sim

#endif // MODEL_H_INCLUDED
