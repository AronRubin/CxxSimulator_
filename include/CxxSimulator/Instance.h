/**
 * Instance.h
 */

#ifndef SIM_MODEL_H_INCLUDED
#define SIM_MODEL_H_INCLUDED

#include "cpp_utils.h"
#include "Clock.h"
#include "Common.h"
#include "Model.h"

#include <optional>
#include <string>
#include <system_error>
#include <memory>
#include <map>
#include <utility>
#include <any>

namespace sim {

class Simulator;
class Simulation;
class Model;
class Activity;
class Instance;
class Pad;

class Pad : public std::enable_shared_from_this<Pad> {
public:
  Pad( std::shared_ptr<Instance> instance, const PadSpec &spec, const std::string &name );
  Pad( Pad &&other );
  Pad &operator=( Pad &&other );
  ~Pad() noexcept;
  
  PadSpec spec() const;
  std::string name() const;
  std::shared_ptr<Instance> owner() const;
  std::shared_ptr<Pad> peer() const;
  size_t available() const;

  bool connect( std::shared_ptr<Instance> instance, const std::string &pad_name );

private:
  class Impl;
  std::unique_ptr<Impl> impl;

public:
  class Private;
};

class Activity : public std::enable_shared_from_this<Activity> {
public:
  enum class State : uint32_t { INIT, RUN, PAUSE, DONE };

  Activity( std::shared_ptr<Instance> instance, const ActivitySpec &spec, const std::string &name, ActivitySpec::PlainFunc &func );
  Activity( std::shared_ptr<Instance> instance, const ActivitySpec &spec, const std::string &name, ActivitySpec::PayloadFunc &func );
  Activity( std::shared_ptr<Instance> instance, const ActivitySpec &spec, const std::string &name, ActivitySpec::PadFunc &func );
  
  Activity( Activity &&other );
  Activity &operator=( Activity &&other );
  ~Activity() noexcept;

  ActivitySpec spec() const;
  State state() const;
  std::shared_ptr<Instance> owner() const;
  std::string name() const;

  /**
   * @brief 
   */
#if ACPP_LESSON > 3
  acpp::value_result<std::any> padReceive( const std::string &pad_name, const std::string &then_activity );
  acpp::value_result<std::any> padReceive( const std::string &pad_name, sim::Clock::duration timeout, const std::string &then_activity );
  bool padSend( const std::string &pad_name, const std::any &payload, const std::string &then_activity );
#endif

#if ACPP_LESSON > 4
  void waitFor( sim::Clock::duration dur );
  void waitUntil( const sim::Clock::time_point &time );
  acpp::value_result<std::any> padReceive( const std::string &pad_name );
  acpp::value_result<std::any> padReceive( const std::string &pad_name, sim::Clock::duration timeout );
  bool padSend( const std::string &pad_name, const std::any &payload, bool block = false );
#endif

private:
  class Impl;
  std::unique_ptr<Impl> impl;
};

class Instance : public std::enable_shared_from_this<Instance> {
public:
  Instance(
      std::shared_ptr<Simulation> sim,
      std::shared_ptr<Model> model,
      const std::string &name,
      const PropertyList &parameters );
  Instance( Instance &&other );
  Instance &operator=( Instance &&other );
  ~Instance() noexcept;

  std::string name() const;
  std::shared_ptr<Model> model() const;
  std::shared_ptr<Simulation> owner() const;
  acpp::unstructured_value parameter( const std::string &name ) const;
  template <typename T>
  std::optional<T> parameter( const std::string &name ) const {
    return acpp::get_as<T>( parameter( name ) );
  }
  std::shared_ptr<Activity> activity( const std::string &name ) const;
  std::vector<std::shared_ptr<Activity>> activities() const;
  std::shared_ptr<Pad> pad( const std::string &name ) const;
  
  acpp::void_result<> setParameter( const std::string &name, const acpp::unstructured_value &value );

  acpp::void_result<> spawnActivity( const std::string spec_name, const std::string &name, Clock::duration delay = {} );

  virtual std::shared_ptr<Activity> makeActivity( const std::string spec_name, const std::string &name );

private:
  class Impl;
  std::unique_ptr<Impl> impl;
  
public:
  class Private;
};


} // namespace sim

#endif // SIM_INSTANCE_H_INCLUDED