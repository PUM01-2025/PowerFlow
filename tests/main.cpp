#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "powerflow/NetworkLoader.hpp"
#include "powerflow/BackwardForwardSweepSolver.hpp"
#include "powerflow/GaussSeidelSolver.hpp"

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
        std::ifstream file("/Users/simonhansson/U3/Kandidat01/examples/test_networks/invalid_command_network.txt");
        CHECK_FALSE(file.fail()); 
        NetworkLoader loader(file);
        REQUIRE_THROWS_AS(loader.loadNetwork() , std::runtime_error);
        REQUIRE_THROWS_WITH(loader.loadNetwork(), Catch::Matchers::Contains("Invalid command"));
    }
   
    SECTION("Invalid edge parent index"){
        std::ifstream file("/Users/simonhansson/U3/Kandidat01/examples/test_networks/invalid_node_index.txt");
        CHECK_FALSE(file.fail());
        NetworkLoader loader(file);
        REQUIRE_THROWS_AS(loader.loadNetwork() , std::runtime_error);
        REQUIRE_THROWS_WITH(loader.loadNetwork(), Catch::Matchers::Contains("Invalid node index"));
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

TEST_CASE("Compare output of BFS and GS","[validation]"){
    //Load BFS
    std::ifstream fileBFS("/Users/simonhansson/U3/Kandidat01/examples/example_network.txt");
    CHECK_FALSE(fileBFS.fail());
    NetworkLoader loaderBFS(fileBFS);
    std::unique_ptr<Network> netBFS = loaderBFS.loadNetwork();
    std::vector<GridSolver*> solversBFS;
    solversBFS.push_back(new BackwardForwardSweepSolver(&netBFS->grids.at(0)));
    for (Grid& grid : netBFS->grids) {
        solversBFS.push_back(new BackwardForwardSweepSolver(&grid));
    }

    //Load GS
    std::ifstream file("/Users/simonhansson/U3/Kandidat01/examples/example_network.txt");
    CHECK_FALSE(file.fail());
    NetworkLoader loader(file);
    std::unique_ptr<Network> net = loader.loadNetwork();
    std::vector<GridSolver*> solvers;
    for (Grid& grid : net->grids) {
        solvers.push_back(new GaussSeidelSolver(&grid));
    }

    //Compare the result
    for( unsigned long  i = 0; i < net->grids.size(); i++){
        for( unsigned long j = 0; j < net->grids[i].nodes.size(); j++){
            REQUIRE(net->grids[0].nodes[j].v.real() == netBFS->grids[0].nodes[j].v.real());
            REQUIRE(net->grids[0].nodes[j].v.imag() == netBFS->grids[0].nodes[j].v.imag());
            REQUIRE(net->grids[0].nodes[j].s.real() == netBFS->grids[0].nodes[j].s.real());
            REQUIRE(net->grids[0].nodes[j].s.imag() == netBFS->grids[0].nodes[j].s.imag());
        }
    }
}