
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

class TestModel : public sim::Model {
public:
  TestModel() : Model( "TestModel" ) {}

  sim::ActivitySpec startActivity() override {
    return { "start", "start", []( sim::Instance &ins, sim::Activity &act ) {} };
  }
};

TEST_F( SimulatorTest, model_adoption ) {
  sim::Simulator::getInstance().addModel<TestModel>();

  EXPECT_TRUE( sim::Simulator::getInstance().model( "TestModel" ) );
}

