

#ifndef SIM_QUEUING_H_INCLUDED
#define SIM_QUEUING_H_INCLUDED

#include <CxxSimulator/Simulator.h>

namespace sim {
namespace queuing {

class SourceModel : public Model {
public:
  SourceModel();
  virtual ~SourceModel() = default;
  SourceModel( SourceModel &&other ) noexcept = default;
  SourceModel &operator=( SourceModel &&other ) noexcept = default;
  SourceModel( SourceModel &other ) = default;
  SourceModel &operator=( SourceModel &other ) = default;

  // entry point for the model
  void startActivity( Instance &instance, Activity &activity ) override;
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
  void startActivity( Instance &instance, Activity &activity ) override;
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
  void startActivity( Instance &instance, Activity &activity ) override;
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
  void startActivity( Instance &instance, Activity &activity ) override;
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
  void startActivity( Instance &instance, Activity &activity ) override;
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
  void startActivity( Instance &instance, Activity &activity ) override;
};

} // namespace queuing
} // namespace sim

#endif // SIM_QUEUING_H_INCLUDED
