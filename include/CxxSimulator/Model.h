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

#if ACPP_LESSON > 3
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
#endif // ACPP_LESSON > 3

/**
 * Structure to specify a how an activity is constructed
 */
struct ActivitySpec {
  enum class Type {
    undefined,
    plain,
#if ACPP_LESSON > 3
    pad_receive,
    pad_send
#endif // ACPP_LESSON > 3
  };

  ActivitySpec() = default; // this will make an invalid/null spec
  ActivitySpec(
      const std::string &name,
      const Type &type ) :
      name( name ),
      type( type ) {}
  ~ActivitySpec() noexcept = default;
  ActivitySpec( const ActivitySpec &other ) = default;
  ActivitySpec( ActivitySpec &&other ) noexcept = default;
  ActivitySpec &operator=( const ActivitySpec &other ) = default;
  ActivitySpec &operator=( ActivitySpec &&other ) noexcept = default;

  std::string name;
  Type type = Type::undefined;
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
  std::vector<ActivitySpec> activities();
  ActivitySpec activity( const std::string &name );
#if ACPP_LESSON > 3
  std::vector<PadSpec> pads();
  PadSpec pad( const std::string &name );
#endif // ACPP_LESSON > 3

  // entry point for the model
  virtual void startActivity( std::shared_ptr<Instance> instance, std::shared_ptr<Activity> activity ) = 0;

  virtual std::shared_ptr<Instance> makeInstance(
      std::shared_ptr<Simulation> sim,
      const std::string &name,
      const PropertyList &parameters );

protected:
  void addActivitySpec( const ActivitySpec &spec );
#if ACPP_LESSON > 3
  void addPadSpec( const PadSpec &spec );
#endif // ACPP_LESSON > 3

private:
  class Impl;
  std::unique_ptr<Impl> impl;
};

}  // namespace sim

#endif // MODEL_H_INCLUDED
