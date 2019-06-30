
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

void SourceModel::startActivity( Instance &instance, Activity &activity ) {
  using namespace std::chrono_literals;
  auto duty_cycle = instance.parameter<double>( "duty_cycle" ).value_or( 2.0 );
  Clock::duration interval { static_cast<Clock::duration::rep>( Clock::period::den / duty_cycle ) };
  while( activity.state() == Activity::State::RUN ) {
    instance.pad( "out" );
    activity.waitFor( interval );
  }
}

QueueModel::QueueModel() : Model("QueueModel") {
  addPadSpec( { "in", { PadSpec::Flag::CAN_INPUT }, {} } );
  addPadSpec( { "out", { PadSpec::Flag::CAN_OUTPUT }, {} } );
}

void QueueModel::startActivity( Instance &instance, Activity &activity ) {
}

ProcessorModel::ProcessorModel() : Model("ProcessorModel") {
  addPadSpec( { "in", { PadSpec::Flag::CAN_INPUT }, {} } );
  addPadSpec( { "out", { PadSpec::Flag::CAN_OUTPUT }, {} } );
}

void ProcessorModel::startActivity( Instance &instance, Activity &activity ) {
}

DelayModel::DelayModel() : Model("DelayModel") {
  addPadSpec( { "in", { PadSpec::Flag::CAN_INPUT }, {} } );
  addPadSpec( { "out", { PadSpec::Flag::CAN_OUTPUT }, {} } );
}

void DelayModel::startActivity( Instance &instance, Activity &activity ) {
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
