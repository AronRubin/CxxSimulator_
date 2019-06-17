
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
}

ActivitySpec SourceModel::startActivity() {
  using namespace std::chrono_literals;
  return {
    "start",
    "start",
    []( Instance &ins, Activity &act ) {
      while( act.state() == Activity::State::RUN ) {
        act.waitFor( 1s );
      }
    }
  };
}

QueueModel::QueueModel() : Model("QueueModel") {
}

ActivitySpec QueueModel::startActivity() {
  return { "start", "start", []( Instance &ins, Activity &act ) { } };
}

ProcessorModel::ProcessorModel() : Model("ProcessorModel") {
}

ActivitySpec ProcessorModel::startActivity() {
  return { "start", "start", []( Instance &ins, Activity &act ) { } };
}

DelayModel::DelayModel() : Model("DelayModel") {
}

ActivitySpec DelayModel::startActivity() {
  return { "start", "start", [this]( Instance &ins, Activity &act ) { } };
}

MultiplexModel::MultiplexModel() : Model("MultiplexModel") {
}

ActivitySpec MultiplexModel::startActivity() {
  return { "start", "start", [this]( Instance &ins, Activity &act ) { } };
}

SinkModel::SinkModel() : Model("SinkModel") {
}

ActivitySpec SinkModel::startActivity() {
  return { "start", "start", [this]( Instance &ins, Activity &act ) { } };
}


} // namespace queuing
} // namespace sim
