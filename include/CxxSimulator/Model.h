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
 * POD to specify a model's pad
 */
struct PadSpec {
  enum class Flag : uint32_t { CAN_INPUT, CAN_OUTPUT, IS_TEMPLATE, BY_REQUEST, COUNT__ };

  std::string name;
  acpp::flagset<Flag> flags = { Flag::CAN_INPUT, Flag::CAN_OUTPUT };
  std::unordered_map<std::string, acpp::unstructured_value> properties;
};

struct ActivitySpec {
  using ActivityFunc = std::function<void( Instance &, Activity & )>;

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
  virtual ActivitySpec startActivity() = 0;
  
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
