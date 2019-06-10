/**
 * Model.h
 */

#pragma once

#include <optional>
#include <string>
#include <system_error>
#include <bitset>
#include <map>
#include "cpp_utils.h"

namespace sim {

class PadSpec {
public:
  enum class Flag : uint32_t { CAN_INPUT, CAN_OUTPUT, IS_TEMPLATE, BY_REQUEST, COUNT__ };

  PadSpec() noexcept = default;
  PadSpec( const std::string &name,
      const acpp::flagset<Flag> &flags = { Flag::CAN_INPUT, Flag::CAN_OUTPUT },
      const std::string properties_json = {} ) {}
  const std::string name;
  const acpp::flagset<Flag> flags;
  const std::unordered_map<std::string, acpp::unstructured_value> properties;
};

class Activity {
  //ActivitySpec  
  
};

class ActivitySpec {
public:
  ActivitySpec( const std::string &name, const std::string &triggering_event ) :
      m_name( name ),
      m_triggering_event( triggering_event ) {}
  std::string getName() const { return m_name; }
  void setName( const std::string &name ) { m_name = name; }
  std::string m_name;
  std::string m_triggering_event;
  std::function<void()> worker;
};

class Instance {
public:
  Instance() = default;

  int idx = 0;
};

/**
 * A model is an instance prototype and factory
 */
class Model {
public:
  Model();
  ~Model();
  Model( Model &&other ) = default;
  Model &operator=( Model &&other ) = default;

  // instance factory
  virtual acpp::value_result<Instance> instance( const std::string &name ) = 0;
  
  // pad spec factory
  void addPadSpec( const PadSpec &spec );

  // pad spec factory
  void addActivitySpec( const PadSpec &spec );

private:
  class Impl;
  std::unique_ptr<Impl> impl;
};

}  // namespace sim
