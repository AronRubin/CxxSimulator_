
#ifdef USE_CATCH2
// Let Catch provide main():
#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>
#else // USE_CATCH2
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#endif // USE_CATCH2

#include <CxxSimulator/Simulator.h>

#if defined(USE_CATCH2)
SCENARIO( "Simulator executing", "[simulator]" ) {
  GIVEN( "A simulator loaded with a simple topology" ) {
    std::vector<int> v( 5 );

    REQUIRE( v.size() == 5 );
    REQUIRE( v.capacity() >= 5 );

    WHEN( "the size is increased" ) {
      v.resize( 10 );

      THEN( "the size and capacity change" ) {
        REQUIRE( v.size() == 10 );
        REQUIRE( v.capacity() >= 10 );
      }
    }
    WHEN( "the size is reduced" ) {
      v.resize( 0 );

      THEN( "the size changes but not capacity" ) {
        REQUIRE( v.size() == 0 );
        REQUIRE( v.capacity() >= 5 );
      }
    }
    WHEN( "more capacity is reserved" ) {
      v.reserve( 10 );

      THEN( "the capacity changes but not the size" ) {
        REQUIRE( v.size() == 5 );
        REQUIRE( v.capacity() >= 10 );
      }
    }
    WHEN( "less capacity is reserved" ) {
      v.reserve( 0 );

      THEN( "neither size nor capacity are changed" ) {
        REQUIRE( v.size() == 5 );
        REQUIRE( v.capacity() >= 5 );
      }
    }
  }
}
#endif // CATCH2

TEST( flagset, metas ) {
  enum class Flags : uint32_t { ONE, TWO, THREE, COUNT__ };
  acpp::flagset<Flags> fs( Flags::ONE | Flags::TWO );
  acpp::flagset<Flags> fs2( Flags::ONE, Flags::TWO );
  // std::cout << fs2 << std::endl;
}

