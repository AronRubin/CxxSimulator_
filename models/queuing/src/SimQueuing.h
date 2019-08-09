

#ifndef SIM_QUEUING_H_INCLUDED
#define SIM_QUEUING_H_INCLUDED

#include <CxxSimulator/Simulator.h>

namespace sim {
namespace queuing {

struct QueueMessage {
  size_t id;
  size_t length;
};

class SourceModel : public Model {
public:
  SourceModel();
  virtual ~SourceModel() = default;
  SourceModel( SourceModel &&other ) noexcept = default;
  SourceModel &operator=( SourceModel &&other ) noexcept = default;
  SourceModel( SourceModel &other ) = default;
  SourceModel &operator=( SourceModel &other ) = default;

  // entry point for the model
  void startActivity( std::shared_ptr<Instance> instance, std::shared_ptr<Activity> activity ) override;
  std::shared_ptr<Instance> makeInstance(
      std::shared_ptr<Simulation> sim,
      const std::string &name,
      const PropertyList &parameters ) override;
};

class QueueModel : public Model {
public:
  QueueModel();
  virtual ~QueueModel();
  QueueModel( QueueModel &&other ) noexcept = default;
  QueueModel &operator=( QueueModel &&other ) noexcept = default;
  QueueModel( QueueModel &other ) = default;
  QueueModel &operator=( QueueModel &other ) = default;

  // entry point for the model
  void startActivity( std::shared_ptr<Instance> instance, std::shared_ptr<Activity> activity ) override;
  std::shared_ptr<Instance> makeInstance(
      std::shared_ptr<Simulation> sim,
      const std::string &name,
      const PropertyList &parameters ) override;

#if ACPP_LESSON < 5
protected:
  void sentActivity( std::shared_ptr<Instance> instance, std::shared_ptr<Activity> activity, Pad &pad );
  void receivedActivity( std::shared_ptr<Instance> instance, std::shared_ptr<Activity> activity, Pad &pad );
#endif // ACPP_LESSON < 5
};

class ProcessorModel : public Model {
public:
  ProcessorModel();
  virtual ~ProcessorModel();
  ProcessorModel( ProcessorModel &&other ) noexcept = default;
  ProcessorModel &operator=( ProcessorModel &&other ) noexcept = default;
  ProcessorModel( ProcessorModel &other ) = default;
  ProcessorModel &operator=( ProcessorModel &other ) = default;

  // entry point for the model
  void startActivity( std::shared_ptr<Instance> instance, std::shared_ptr<Activity> activity ) override;
  std::shared_ptr<Instance> makeInstance(
      std::shared_ptr<Simulation> sim,
      const std::string &name,
      const PropertyList &parameters ) override;
};

class DelayModel : public Model {
public:
  DelayModel();
  virtual ~DelayModel();
  DelayModel( DelayModel &&other ) noexcept = default;
  DelayModel &operator=( DelayModel &&other ) noexcept = default;
  DelayModel( DelayModel &other ) = default;
  DelayModel &operator=( DelayModel &other ) = default;

  // entry point for the model
  void startActivity( std::shared_ptr<Instance> instance, std::shared_ptr<Activity> activity ) override;
  std::shared_ptr<Instance> makeInstance(
      std::shared_ptr<Simulation> sim,
      const std::string &name,
      const PropertyList &parameters ) override;
};

class MultiplexModel : public Model {
public:
  MultiplexModel();
  virtual ~MultiplexModel();
  MultiplexModel( MultiplexModel &&other ) noexcept = default;
  MultiplexModel &operator=( MultiplexModel &&other ) noexcept = default;
  MultiplexModel( MultiplexModel &other ) = default;
  MultiplexModel &operator=( MultiplexModel &other ) = default;

  // entry point for the model
  void startActivity( std::shared_ptr<Instance> instance, std::shared_ptr<Activity> activity ) override;
  std::shared_ptr<Instance> makeInstance(
      std::shared_ptr<Simulation> sim,
      const std::string &name,
      const PropertyList &parameters ) override;
};

class SinkModel : public Model {
public:
  SinkModel();
  virtual ~SinkModel();
  SinkModel( SinkModel &&other ) noexcept = default;
  SinkModel &operator=( SinkModel &&other ) noexcept = default;
  SinkModel( SinkModel &other ) = default;
  SinkModel &operator=( SinkModel &other ) = default;

  // entry point for the model
  void startActivity( std::shared_ptr<Instance> instance, std::shared_ptr<Activity> activity ) override;
  std::shared_ptr<Instance> makeInstance(
      std::shared_ptr<Simulation> sim,
      const std::string &name,
      const PropertyList &parameters ) override;
};

} // namespace queuing
} // namespace sim

#endif // SIM_QUEUING_H_INCLUDED
