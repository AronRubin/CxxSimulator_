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
  Pad();
  Pad( Pad &&other ) noexcept;
  Pad &operator=( Pad &&other ) noexcept;
  ~Pad();
  
  PadSpec spec() const;
  std::string name() const;
  std::shared_ptr<Instance> owner() const;
  std::shared_ptr<Pad> peer() const;

  bool connect( std::shared_ptr<Instance> instance, const std::string &pad_name );
  
  size_t available() const;
  acpp::value_result<std::any> receive( const std::string &name );
  acpp::value_result<std::any> receive( const std::string &name, sim::Clock::duration timeout );
  acpp::void_result<> send( const std::any &payload );
  acpp::void_result<> send( std::any &&payload );

private:
  class Impl;
  std::unique_ptr<Impl> impl;
};

class Activity : public std::enable_shared_from_this<Activity> {
public:
  enum class State : uint32_t { INIT, RUN, PAUSE, DONE };

  Activity( std::shared_ptr<Instance> instance, const ActivitySpec &spec, const std::string &name );
  ~Activity();
  Activity( Activity &&other ) noexcept;
  Activity &operator=( Activity &&other ) noexcept;

  ActivitySpec spec() const;
  State state() const;
  std::shared_ptr<Instance> owner() const;
  std::string name() const;

  void waitFor( sim::Clock::duration dur );
  void waitUntil( const sim::Clock::time_point &time );
  bool waitOn( const std::string &signal_name );
  bool waitOn( const std::string &signal_name, sim::Clock::duration timeout );

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
  Instance( Instance &&other ) noexcept;
  Instance &operator=( Instance &&other ) noexcept;
  ~Instance();

  std::string name() const;
  std::shared_ptr<Model> model() const;
  acpp::unstructured_value parameter( const std::string &name ) const;
  template <typename T>
  std::optional<T> parameter( const std::string &name ) const {
    return acpp::get_as<T>( parameter( name ) );
  }
  std::shared_ptr<Activity> activity( const std::string &name ) const;
  std::vector<std::shared_ptr<Activity>> activities() const;
  std::shared_ptr<Pad> pad( const std::string &name ) const;

  std::shared_ptr<Activity> requestActivity( const std::string spec_name, const std::string &name );
  acpp::void_result<> setParameter( const std::string &name, const acpp::unstructured_value &value );

  std::shared_ptr<Simulation> owner() const;

private:
  class Impl;
  std::unique_ptr<Impl> impl;
  
public:
  class Private;
};


} // namespace sim

#endif // SIM_INSTANCE_H_INCLUDED