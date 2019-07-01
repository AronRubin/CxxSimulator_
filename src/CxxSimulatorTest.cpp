
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <CxxSimulator/Simulator.h>

TEST( flagset, metas ) {
  enum class Flags : uint32_t { ONE, TWO, THREE, COUNT__ };
  acpp::flagset<Flags> fs2( Flags::ONE, Flags::TWO );
  EXPECT_TRUE( fs2[Flags::ONE] );
  EXPECT_TRUE( fs2[Flags::TWO] );
  EXPECT_FALSE( fs2[Flags::THREE] );
}

TEST( val_in, metas ) {
  EXPECT_TRUE( acpp::val_in( 1, 1, 2, 3, 4 ) );
  EXPECT_FALSE( acpp::val_in( 5, 1, 2, 3, 4 ) );
}

/**
 * Fixture for testing the simulator which resets the global simulator instance for each test.
 */
struct SimulatorTest : testing::Test {
  void SetUp() override {
    sim::Simulator::getInstance().reset();
  }
 
  void TearDown() override {

  }
};

class EmptyModel : public sim::Model {
public:
  EmptyModel() : Model( "EmptyModel" ) {
  }

  void startActivity( sim::Instance &, sim::Activity & ) override {
  }
};

TEST_F( SimulatorTest, model_adoption ) {
  sim::Simulator::getInstance().addModel<EmptyModel>();

  EXPECT_TRUE( sim::Simulator::getInstance().model( "EmptyModel" ) );
}

TEST_F( SimulatorTest, instancing ) {
  sim::Simulator::getInstance().addModel<EmptyModel>();

  EXPECT_TRUE( sim::Simulator::getInstance().model( "EmptyModel" ) );

  sim::Simulation sim;

  sim.spawnInstance( "EmptyModel", "inst" );

  sim.setState( sim::Simulation::State::RUN );
}

/**
 * Fixture for testing the simulator which resets the global simulator instance for each test.
 */
struct SimulationTest : testing::Test {
  class LoopbackModel : public sim::Model {
  public:
    LoopbackModel() : Model( "LoopbackModel" ) {
      addActivitySpec( {"sink",
          []( sim::Instance &instance, sim::Activity &activity ) {
            while ( activity.state() == sim::Activity::State::RUN ) {
              activity.padReceive( "in" );
            }
          },
          "in::receive"} );
      addPadSpec( {"in", {sim::PadSpec::Flag::CAN_INPUT}, {}} );
      addPadSpec( {"out", {sim::PadSpec::Flag::CAN_OUTPUT}, {}} );
    }

    void startActivity( sim::Instance &instance, sim::Activity &activity ) override {
      auto duty_cycle = instance.parameter<double>( "duty_cycle" ).value_or( 2.0 );
      sim::Clock::duration interval{
          static_cast<sim::Clock::duration::rep>( sim::Clock::period::den / duty_cycle )};
      while ( activity.state() == sim::Activity::State::RUN ) {
        instance.pad( "out" );
        activity.waitFor( interval );
      }
    }
  };

  void SetUp() override {
    auto &simulator = sim::Simulator::getInstance();
    simulation.reset();
    simulator.reset();
    simulator.addModel<LoopbackModel>();
    simulation = std::make_shared<sim::Simulation>();
  }

  void TearDown() override {
  }

  std::shared_ptr<sim::Simulation> simulation;
};

TEST_F( SimulationTest, instance_basic ) {
  simulation->spawnInstance( "LoopbackModel", "looper" );

  simulation->setState( sim::Simulation::State::RUN );
}
