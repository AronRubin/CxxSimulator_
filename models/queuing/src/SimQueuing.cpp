
#include "SimQueuing.h"

namespace sim {
namespace queuing {

struct ModelRegistrar {
  ModelRegistrar() {
    Simulator &simulator = Simulator::getInstance();
    simulator.addModel<SourceModel>();
    simulator.addModel<QueueModel>();
    simulator.addModel<ProcessorModel>();
    simulator.addModel<DelayModel>();
    simulator.addModel<MultiplexModel>();
    simulator.addModel<SinkModel>();
  }
};

static ModelRegistrar simQueuingRegistrar;

SourceModel::SourceModel() : Model("SourceModel") {
  addPadSpec( { "out", { PadSpec::Flag::CAN_OUTPUT }, {} } );
}

struct SourceModelInstance : public Instance {
  SourceModelInstance(
      std::shared_ptr<Simulation> sim,
      std::shared_ptr<Model> model,
      const std::string &name,
      const PropertyList &parameters ) :
      Instance{ sim, model, name, parameters } {
    duty_cycle = parameter<double>( "duty_cycle" ).value_or( 2.0 );
  }

  void start( std::shared_ptr<Instance> instance, std::shared_ptr<Activity> activity );

  double duty_cycle = 2.0;
};

std::shared_ptr<Instance> SourceModel::makeInstance(
    std::shared_ptr<Simulation> sim,
    const std::string &name,
    const PropertyList &parameters ) {
  auto instance = std::make_shared<SourceModelInstance>( sim, shared_from_this(), name, parameters );
  ActivitySpec startSpec { "start", };
}

#if ACPP_LESSON > 4
void SourceModel::startActivity( std::shared_ptr<Instance> instance, std::shared_ptr<Activity> activity ) {
  auto duty_cycle = instance->parameter<double>( "duty_cycle" ).value_or( 2.0 );
  Clock::duration interval { static_cast<Clock::duration::rep>( Clock::period::den / duty_cycle ) };
  
  while (activity->state() == Activity::State::RUN) {
    activity->padSend( "out", std::make_any<QueueMessage>( 0, 1 ) );
    activity->waitFor( interval );
  }
}
#else // ACPP_LESSON <= 4

#endif // ACPP_LESSON <= 4
QueueModel::QueueModel() : Model("QueueModel") {
  addPadSpec( { "in", { PadSpec::Flag::CAN_INPUT }, {} } );
  addPadSpec( { "out", { PadSpec::Flag::CAN_OUTPUT }, {} } );
}

void QueueModel::startActivity( Instance &instance, Activity &activity ) {
  auto queue_depth = instance.parameter<size_t>( "depth" ).value_or( 1 );
  auto out = instance.pad( "out" );
  auto out_peer = out->peer();
  if (!out_peer) {
    return;
  }
  if (out_peer->available() < queue_depth) {
    auto received = activity.padReceive( "in" );
    if (received) {
      activity.padSend( "out", received.value );
    }
  }
}

ProcessorModel::ProcessorModel() : Model("ProcessorModel") {
  addPadSpec( { "in", { PadSpec::Flag::CAN_INPUT }, {} } );
  addPadSpec( { "out", { PadSpec::Flag::CAN_OUTPUT }, {} } );
}

void ProcessorModel::startActivity( Instance &instance, Activity &activity ) {
  auto rate = instance.parameter<double>( "rate" ).value_or( 1.0 );
  if (m_received.has_value() ) {
    // woke up after a delay
    activity.padSend( "out", std::make_any<QueueMessage>( *m_received ) );
    m_received.reset();
  }
  auto received = activity.padReceive( "in" );
  if (received && received.value.value().type() == typeid(QueueMessage)) {
    m_received.emplace( std::any_cast<QueueMessage &&>( std::move( received.value.value() ) ) );
    Clock::duration execution_delay { static_cast<Clock::duration::rep>( Clock::period::den / duty_cycle ) };

    auto execution_delay = std::chrono::duration_cast<Clock::duration>( std::chrono::duration<double>( m_received->length * rate ) );
    activity.waitFor( execution_delay );
  }
}

DelayModel::DelayModel() : Model("DelayModel") {
  addPadSpec( { "in", { PadSpec::Flag::CAN_INPUT }, {} } );
  addPadSpec( { "out", { PadSpec::Flag::CAN_OUTPUT }, {} } );
}

void DelayModel::startActivity( Instance &instance, Activity &activity ) {
  auto rate = instance.parameter<double>( "rate" ).value_or( 1.0 );
  if (m_received.has_value() ) {
    // woke up after a delay
    activity.padSend( "out", std::make_any<QueueMessage>( *m_received ) );
    m_received.reset();
  }
  auto received = activity.padReceive( "in" );
  if (received && received.value.value().type() == typeid(QueueMessage)) {
    m_received.emplace( std::any_cast<QueueMessage &&>( std::move( received.value.value() ) ) );
    auto execution_delay = std::chrono::duration_cast<Clock::duration>( std::chrono::duration<double>( m_received->length * rate ) );
    activity.waitFor( execution_delay );
  }
}

MultiplexModel::MultiplexModel() : Model("MultiplexModel") {
  addPadSpec( { "in", { PadSpec::Flag::CAN_INPUT }, {} } );
  addPadSpec( { "out", { PadSpec::Flag::CAN_OUTPUT, PadSpec::Flag::BY_REQUEST }, {} } );
}

void MultiplexModel::startActivity( Instance &instance, Activity &activity ) {
}

SinkModel::SinkModel() : Model("SinkModel") {
  addPadSpec( { "in", { PadSpec::Flag::CAN_INPUT }, {} } );
}

void SinkModel::startActivity( Instance &instance, Activity &activity ) {
}


} // namespace queuing
} // namespace sim
