/**
 * Model.h
 */

#ifndef MODEL_H_INCLUDED
#define MODEL_H_INCLUDED

#include <optional>
#include <string>
#include <system_error>
#include <bitset>
#include <map>
#include "cpp_utils.h"

namespace sim {

class Simulator;
class Model;

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
  std::string m_name;
  std::string m_triggering_event;
  std::function<void()> m_function;
};

class Activity : public std::enable_shared_from_this<Activity> {
public:
  enum class State : uint32_t { INIT, RUN, PAUSE, DONE };

  Activity( const ActivitySpec &spec );

  ActivitySpec spec() const;
  
  State state() const;

  State state( State &pending ) const;

private:
  class Impl;
  std::unique_ptr<Impl> impl;
};

class Instance : public std::enable_shared_from_this<Instance> {
public:
  std::string name() const;
  std::shared_ptr<Model> model() const;
  acpp::unstructured_value parameter( const std::string &name ) const;
  std::shared_ptr<Activity> activity( const std::string &name ) const;

  std::shared_ptr<Activity> requestActivity( const std::string spec_name, const std::string &name );
  acpp::void_result<> setParameter( const std::string &name, const acpp::unstructured_value &value );

private:
  // only simulator can create an instance
  friend class Simulator;
  Instance( Simulator &sim, std::shared_ptr<Model> model, const std::string &name );

  class Impl;
  std::unique_ptr<Impl> impl;
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
