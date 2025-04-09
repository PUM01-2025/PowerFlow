#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "powerflow/NetworkLoader.hpp"

#include <fstream>
#include <string>

int add(int a,int b){
    return a+b;
}


// TEST_CASE("DEBUG", "[debug]"){
//     REQUIRE(add(5,5) == 1);
// }

bool hasEnding (std::string const &fullString, std::string const &ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}


//CHECK_FALSE(file.fail());  checks that the file can be opened correctly (in most cases it is the wrong filepath)
TEST_CASE("Networkloader input", "[!throws]" ) {
    
    SECTION("Invalid command"){
        std::ifstream file("/Users/simonhansson/U3/Kandidat01/examples/invalid_command_network.txt");
        CHECK_FALSE(file.fail()); 
        NetworkLoader loader(file);
        REQUIRE_THROWS_AS(loader.loadNetwork() , std::runtime_error);
        REQUIRE_THROWS_WITH(loader.loadNetwork(), Catch::Matchers::Contains("Invalid command"));
    }

    /*
    #These tests are deprecated as the cover cases that can't be reached

    SECTION("Invalid node count"){
        std::ifstream file("/Users/simonhansson/U3/Kandidat01/examples/invalid_node_count_network.txt");
        CHECK_FALSE(file.fail()); 
        NetworkLoader loader(file);
        loader.loadNetwork();

        REQUIRE_THROWS_AS(loader.loadNetwork() , std::runtime_error);
        REQUIRE_THROWS_WITH(loader.loadNetwork(), Catch::Matchers::Contains("Invalid node count"));
    }

    SECTION("Missing end of list (%)"){
        std::ifstream file("/Users/simonhansson/U3/Kandidat01/examples/missing_end_list_network.txt");
        CHECK_FALSE(file.fail()); 
        NetworkLoader loader(file);
        loader.loadNetwork();

        REQUIRE_THROWS_AS(loader.loadNetwork() , std::runtime_error);
        REQUIRE_THROWS_WITH(loader.loadNetwork(), Catch::Matchers::Contains("Missing end-of-list indicator"));
    }
    */
}