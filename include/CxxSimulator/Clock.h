
#ifndef CLOCK_H_INCLUDED
#define CLOCK_H_INCLUDED

#include <chrono>

namespace sim {

/**
 *  @brief Clock that captures simulation time (Monotonic)
 *  Concept: TrivialClock
 *  Time returned has the property of only increasing at a uniform rate from
 *  simulation start (time 0).
 */
class Clock {
public:
  using duration = std::chrono::nanoseconds;
  using rep = duration::rep;
  using period = duration::period;
  using time_point = std::chrono::time_point<Clock, duration>;

  static constexpr bool is_steady = true;

  time_point now() noexcept;

private:
  // only the simulation should manage this clock
  friend class Simulation;
  time_point m_tp;
};

} // namespace sim

#endif // CLOCK_H_INCLUDED
