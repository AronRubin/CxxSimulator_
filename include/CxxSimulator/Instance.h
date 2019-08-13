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
class Instance;

#if ACPP_LESSON > 3
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

  /**
   * @brief Connect this pad to a peer on another instance
   * @param instance the peer instance
   * @param pad_name the peer pad
   * @return success
   */
  bool connect( std::shared_ptr<Instance> instance, const std::string &pad_name );

private:
  class Impl;
  std::unique_ptr<Impl> impl;

public:
  class Private;
};
#endif // ACPP_LESSON > 3

class Activity : public std::enable_shared_from_this<Activity> {
public:
  using Func = std::function<void( Instance &, Activity &, const std::string &source, const std::any &payload )>;
#if ACPP_LESSON > 4
  enum class State : uint32_t { init, run, pause, done };
#endif // ACPP_LESSON > 4

  Activity(
    std::shared_ptr<Instance> instance,
    const ActivitySpec &spec,
    const std::string &name,
    const Func &func );

  Activity( Activity &&other ) noexcept;
  Activity &operator=( Activity &&other ) noexcept;
  ~Activity() noexcept;

  ActivitySpec spec() const;
  /**
   * @brief Invoke the activity
   * @param source the source of the activity's invocation
   * @param payload a payload to pass to the activity
   */
  void invoke( const std::string &source, const std::any &payload );

#if ACPP_LESSON > 4
  State state() const;
  void pause();
  void done();
#endif // ACPP_LESSON > 4
  std::shared_ptr<Instance> owner() const;
  std::string name() const;

  /**
   * The following are meant for the activity itself to call
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
  Instance( Instance &&other ) noexcept;
  Instance &operator=( Instance &&other ) noexcept;
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
  /**
   * @brief Request an activity to be spawned on this instance by the simulator 
   * @param spec_name The name of the activity's spec
   * @param name The name of the activity (must be unique amoung spawned activities for this instance)
   * @param delay The simulation delay, if any, before spawning the activity
   * @return acpp::void_result<> Success or error indicator
   */
  acpp::void_result<> spawnActivity( const std::string spec_name, const std::string &name, Clock::duration delay = {} );
  /**
   * @brief Add an activity to this instance without spawning it.
   * @param spec_name The name of the activity's spec
   * @param name The name of the activity (must be unique amoung spawned activities for this instance)
   * @return std::shared_ptr<Activity> the activity added or nullptr on failure
   */
  std::shared_ptr<Activity> addActivity( const std::string &spec_name, const std::string &name );
  /**
   * @brief Extensible function for making an activity for this instance. Called by addActivity.
   * @param spec The name of the activity's spec
   * @param name The name of the activity
   * @return std::shared_ptr<Activity> the activity made or nullptr on failure
   */
  virtual std::shared_ptr<Activity> makeActivity( const ActivitySpec &spec, const std::string &name );

private:
  class Impl;
  std::unique_ptr<Impl> impl;
};


} // namespace sim

#endif // SIM_INSTANCE_H_INCLUDED