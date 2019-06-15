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

class ActivitySpec {
public:
  ActivitySpec( const std::string &name, const std::string &triggering_event ) :
      m_name( name ),
      m_triggering_event( triggering_event ) {}
  std::string name() const { return m_name; }
  void setName( const std::string &name ) { m_name = name; }

private:
  std::string m_name;
  std::string m_triggering_event;
  std::function<void()> m_function;
};

class Activity : public std::enable_shared_from_this<Activity> {
public:
  Activity();
  
  ActivitySpec spec();
  
private:
  class Impl;
  std::unique_ptr<Impl> impl;
};

class Instance : public std::enable_shared_from_this<Instance> {
public:
  Instance();

  std::string name();
  std::shared_ptr<Model> model();
  acpp::unstructured_value parameter( const std::string &name );
  std::shared_ptr<Activity> activity( const std::string &name );

  acpp::void_result<> setParameter( const std::string &name, const acpp::unstructured_value &value );

private:
  class Impl;
  std::unique_ptr<Impl> impl;
};

/**
 * A model is an instance prototype and factory
 */
class Model : public std::enable_shared_from_this<Model> {
public:
  Model();
  ~Model();
  Model( Model &&other ) = default;
  Model &operator=( Model &&other ) = default;

  // instance factory
  virtual acpp::value_result<Instance> instance( const std::string &name ) = 0;
  
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
