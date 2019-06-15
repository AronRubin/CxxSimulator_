
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
    return { "start", "start", []() {} };
  }
};

TEST_F( SimulatorTest, model_adoption ) {
  auto model = std::make_shared<TestModel>();
  
  sim::Simulator::getInstance().addModel( {} );
}

