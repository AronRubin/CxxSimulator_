/**
 * Model.h
 */

#pragma once

#include <optional>
#include <string>
#include <system_error>
#include <bitset>
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
  const std::string properties_json;
};

class ActivitySpec {
public:
  std::string name;
  std::string triggering_event;
  std::string properties_json;
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
