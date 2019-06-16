
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
  return { "start", "start", []() { } };
}

QueueModel::QueueModel() : Model("QueueModel") {
}

ActivitySpec QueueModel::startActivity() {
  return { "start", "start", []() { } };
}

ProcessorModel::ProcessorModel() : Model("ProcessorModel") {
}

ActivitySpec ProcessorModel::startActivity() {
  return { "start", "start", []() { } };
}

DelayModel::DelayModel() : Model("DelayModel") {
}

ActivitySpec DelayModel::startActivity() {
  return { "start", "start", []() { } };
}

MultiplexModel::MultiplexModel() : Model("MultiplexModel") {
}

ActivitySpec MultiplexModel::startActivity() {
  return { "start", "start", []() { } };
}

SinkModel::SinkModel() : Model("SinkModel") {
}

ActivitySpec SinkModel::startActivity() {
  return { "start", "start", []() { } };
}


} // namespace queuing
} // namespace sim
