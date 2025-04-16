#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "powerflow/NetworkLoader.hpp"
#include "powerflow/BackwardForwardSweepSolver.hpp"
#include "powerflow/GaussSeidelSolver.hpp"
#include "powerflow/PowerFlowSolver.hpp"

#include <fstream>
#include <string>


//CHECK_FALSE(file.fail());  checks that the file can be opened correctly (in most cases it is the wrong filepath)
TEST_CASE("Networkloader input", "[!throws]" ) {
    
    SECTION("Invalid command"){
        std::ifstream file("/Users/simonhansson/U3/Kandidat01/examples/test_networks/invalid_command_network.txt");
        CHECK_FALSE(file.fail()); 
        NetworkLoader loader(file);
        REQUIRE_THROWS_AS(loader.loadNetwork() , std::runtime_error);
        REQUIRE_THROWS_WITH(loader.loadNetwork(), Catch::Matchers::Contains("Invalid command"));
    }
   
    // SECTION("Invalid edge parent index"){
    //     std::ifstream file("/Users/simonhansson/U3/Kandidat01/examples/test_networks/invalid_node_index.txt");
    //     CHECK_FALSE(file.fail());
    //     NetworkLoader loader(file);
    //     REQUIRE_THROWS_AS(loader.loadNetwork() , std::runtime_error);
    //     REQUIRE_THROWS_WITH(loader.loadNetwork(), Catch::Matchers::Contains("Invalid node index"));
    // }
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

    SECTION("example_network.txt"){
        //Load BFS
        std::ifstream fileBFS("/Users/simonhansson/U3/Kandidat01/examples/example_network.txt"); //Ladda in testfil
        CHECK_FALSE(fileBFS.fail());                                            //Kommer ge en varning att om det blir fel i filinläsningen
        NetworkLoader loaderBFS(fileBFS);                                       //Skapa en loader
        std::unique_ptr<Network> netBFS = loaderBFS.loadNetwork();              //Ladda in nätveket
        std::vector<GridSolver*> solversBFS;                                    //Vector att spara läsarna i
        for (Grid& grid : netBFS->grids) {                                      //Loopa igenom nätet och lätt till en lösare för varje subnät
            solversBFS.push_back(new BackwardForwardSweepSolver(&grid));
        }

        //Ladda in effektbelastningar
        // 2 l (0.004, 0.002)
        // 1 l (0.002, 0.001)
        // 2 l (0.005, 0.004)
        netBFS->grids.at(1).nodes.at(2).s = -complex_t(0.004,0.002);
        netBFS->grids.at(2).nodes.at(1).s = -complex_t(0.002,0.001);
        netBFS->grids.at(2).nodes.at(2).s = -complex_t(0.005, 0.004);

        //Loopa över alla lösare och kör dem
        for (GridSolver* solverBFS : solversBFS) {
            solverBFS->solve();
        }
        
        //Load GS //Samma som ovan men för GaussSeidel
        std::ifstream file("/Users/simonhansson/U3/Kandidat01/examples/example_network.txt");
        CHECK_FALSE(file.fail());
        NetworkLoader loader(file);
        std::unique_ptr<Network> net = loader.loadNetwork();
        std::vector<GridSolver*> solvers;
        for (Grid& grid : net->grids) {
            solvers.push_back(new GaussSeidelSolver(&grid));
        }

        // 2 l (0.004, 0.002)
        // 1 l (0.002, 0.001)
        // 2 l (0.005, 0.004)
        net->grids.at(1).nodes.at(2).s = -complex_t(0.004,0.002);
        net->grids.at(2).nodes.at(1).s = -complex_t(0.002,0.001);
        net->grids.at(2).nodes.at(2).s = -complex_t(0.005, 0.004);

        for (GridSolver* solver : solvers) {
            solver->solve();
        }
    
        //Compare the result
        for( unsigned long  i = 0; i < net->grids.size(); i++){
            for( unsigned long j = 0; j < net->grids[i].nodes.size(); j++){
                if(net->grids[i].nodes[j].type == NodeType::MIDDLE){
                    continue;
                }
                //Kollar att GS svaret är inom 0.000001 av BFS svaret med hjälp av en catch2 matcher
                CHECK_THAT(net->grids[i].nodes[j].v.real(), Catch::Matchers::WithinAbs(netBFS->grids[i].nodes[j].v.real(), 0.000001));
                CHECK_THAT(net->grids[i].nodes[j].v.imag(), Catch::Matchers::WithinAbs(netBFS->grids[i].nodes[j].v.imag(), 0.000001));
                CHECK_THAT(net->grids[i].nodes[j].s.real(), Catch::Matchers::WithinAbs(netBFS->grids[i].nodes[j].s.real(), 0.000001));
                CHECK_THAT(net->grids[i].nodes[j].s.imag(), Catch::Matchers::WithinAbs(netBFS->grids[i].nodes[j].s.imag(), 0.000001));
            }
        }
    }
}

TEST_CASE("Compare treestructure","[validation]"){

    SECTION("BackwardForwardSweepSolver"){
        //Load normal network
        std::ifstream file("/Users/simonhansson/U3/Kandidat01/examples/test_networks/test_network.txt");
        CHECK_FALSE(file.fail());
        NetworkLoader loader(file);
        std::unique_ptr<Network> net = loader.loadNetwork();
        std::vector<GridSolver*> solvers;
        for (Grid& grid : net->grids) {
            solvers.push_back(new BackwardForwardSweepSolver(&grid));
        }

        // 2 l (0.004, 0.002)
        // 1 l (0.002, 0.001)
        // 2 l (0.005, 0.004)
        net->grids.at(1).nodes.at(2).s = -complex_t(0.004,0.002);
        net->grids.at(2).nodes.at(1).s = -complex_t(0.002,0.001);
        net->grids.at(2).nodes.at(2).s = -complex_t(0.005, 0.004);
        for (GridSolver* solverBFS : solvers) {
            solverBFS->solve();
            
        }
        
        //Load Single grid
        std::ifstream fileSG("/Users/simonhansson/U3/Kandidat01/examples/test_networks/test_network_single_grid.txt");
        CHECK_FALSE(fileSG.fail());
        NetworkLoader loaderSG(fileSG);
        std::unique_ptr<Network> netSG = loaderSG.loadNetwork();
        std::vector<GridSolver*> solversSG;
        for (Grid& grid : netSG->grids) {
            solversSG.push_back(new BackwardForwardSweepSolver(&grid));
        }

        // 2 l (0.004, 0.002)
        // 1 l (0.002, 0.001)
        // 2 l (0.005, 0.004)
        netSG->grids.at(0).nodes.at(7).s = -complex_t(0.004,0.002);
        netSG->grids.at(0).nodes.at(5).s = -complex_t(0.002,0.001);
        netSG->grids.at(0).nodes.at(6).s = -complex_t(0.005, 0.004);

        for (GridSolver* solver : solversSG) {
            solver->solve();
        }

        std::cout << "net: " << std::endl;
        for (const Grid& grid : net->grids) {
            for (const GridNode& node : grid.nodes) {
                std::cout << node.v.real() << "," << node.v.imag() << "  " << node.s.real() << "," << node.s.imag() << std::endl;
            }
        }

        std::cout << "SG: " << std::endl;
        for (const Grid& grid : netSG->grids) {
            for (const GridNode& node : grid.nodes) {
                std::cout << node.v.real() << "," << node.v.imag() << "  " << node.s.real() << "," << node.s.imag() << std::endl;
            }
        }
    
        //Compare the result real
        CHECK_THAT(netSG->grids[0].nodes[1].v.real(), Catch::Matchers::WithinAbs(net->grids[0].nodes[1].v.real(), 0.000001));
        CHECK_THAT(netSG->grids[0].nodes[2].v.real(), Catch::Matchers::WithinAbs(net->grids[0].nodes[2].v.real(), 0.000001));
        CHECK_THAT(netSG->grids[0].nodes[3].v.real(), Catch::Matchers::WithinAbs(net->grids[0].nodes[3].v.real(), 0.000001));
        CHECK_THAT(netSG->grids[0].nodes[4].v.real(), Catch::Matchers::WithinAbs(net->grids[1].nodes[1].v.real(), 0.000001));
        CHECK_THAT(netSG->grids[0].nodes[5].v.real(), Catch::Matchers::WithinAbs(net->grids[2].nodes[1].v.real(), 0.000001));
        CHECK_THAT(netSG->grids[0].nodes[6].v.real(), Catch::Matchers::WithinAbs(net->grids[2].nodes[2].v.real(), 0.000001));
        CHECK_THAT(netSG->grids[0].nodes[7].v.real(), Catch::Matchers::WithinAbs(net->grids[1].nodes[2].v.real(), 0.000001));

        CHECK_THAT(netSG->grids[0].nodes[1].v.imag(), Catch::Matchers::WithinAbs(net->grids[0].nodes[1].v.imag(), 0.000001));
        CHECK_THAT(netSG->grids[0].nodes[2].v.imag(), Catch::Matchers::WithinAbs(net->grids[0].nodes[2].v.imag(), 0.000001));
        CHECK_THAT(netSG->grids[0].nodes[3].v.imag(), Catch::Matchers::WithinAbs(net->grids[0].nodes[3].v.imag(), 0.000001));
        CHECK_THAT(netSG->grids[0].nodes[4].v.imag(), Catch::Matchers::WithinAbs(net->grids[1].nodes[1].v.imag(), 0.000001));
        CHECK_THAT(netSG->grids[0].nodes[5].v.imag(), Catch::Matchers::WithinAbs(net->grids[2].nodes[1].v.imag(), 0.000001));
        CHECK_THAT(netSG->grids[0].nodes[6].v.imag(), Catch::Matchers::WithinAbs(net->grids[2].nodes[2].v.imag(), 0.000001));
        CHECK_THAT(netSG->grids[0].nodes[7].v.imag(), Catch::Matchers::WithinAbs(net->grids[1].nodes[2].v.imag(), 0.000001));

        CHECK_THAT(netSG->grids[0].nodes[1].s.real(), Catch::Matchers::WithinAbs(net->grids[0].nodes[1].s.real(), 0.000001));
        CHECK_THAT(netSG->grids[0].nodes[2].s.real(), Catch::Matchers::WithinAbs(net->grids[0].nodes[2].s.real(), 0.000001));
        CHECK_THAT(netSG->grids[0].nodes[3].s.real(), Catch::Matchers::WithinAbs(net->grids[0].nodes[3].s.real(), 0.000001));
        CHECK_THAT(netSG->grids[0].nodes[4].s.real(), Catch::Matchers::WithinAbs(net->grids[1].nodes[1].s.real(), 0.000001));
        CHECK_THAT(netSG->grids[0].nodes[5].s.real(), Catch::Matchers::WithinAbs(net->grids[2].nodes[1].s.real(), 0.000001));
        CHECK_THAT(netSG->grids[0].nodes[6].s.real(), Catch::Matchers::WithinAbs(net->grids[2].nodes[2].s.real(), 0.000001));
        CHECK_THAT(netSG->grids[0].nodes[7].s.real(), Catch::Matchers::WithinAbs(net->grids[1].nodes[2].s.real(), 0.000001));

        CHECK_THAT(netSG->grids[0].nodes[1].s.imag(), Catch::Matchers::WithinAbs(net->grids[0].nodes[1].s.imag(), 0.000001));
        CHECK_THAT(netSG->grids[0].nodes[2].s.imag(), Catch::Matchers::WithinAbs(net->grids[0].nodes[2].s.imag(), 0.000001));
        CHECK_THAT(netSG->grids[0].nodes[3].s.imag(), Catch::Matchers::WithinAbs(net->grids[0].nodes[3].s.imag(), 0.000001));
        CHECK_THAT(netSG->grids[0].nodes[4].s.imag(), Catch::Matchers::WithinAbs(net->grids[1].nodes[1].s.imag(), 0.000001));
        CHECK_THAT(netSG->grids[0].nodes[5].s.imag(), Catch::Matchers::WithinAbs(net->grids[2].nodes[1].s.imag(), 0.000001));
        CHECK_THAT(netSG->grids[0].nodes[6].s.imag(), Catch::Matchers::WithinAbs(net->grids[2].nodes[2].s.imag(), 0.000001));
        CHECK_THAT(netSG->grids[0].nodes[7].s.imag(), Catch::Matchers::WithinAbs(net->grids[1].nodes[2].s.imag(), 0.000001));
        
    }

    SECTION("GaussSeidel"){
        return;
        //Load normal network
        std::ifstream file("/Users/simonhansson/U3/Kandidat01/examples/test_networks/test_network.txt");
        CHECK_FALSE(file.fail());
        NetworkLoader loader(file);
        std::unique_ptr<Network> net = loader.loadNetwork();
        std::vector<GridSolver*> solvers;
        for (Grid& grid : net->grids) {
            solvers.push_back(new GaussSeidelSolver(&grid));
        }

        // 2 l (0.004, 0.002)
        // 1 l (0.002, 0.001)
        // 2 l (0.005, 0.004)
        net->grids.at(1).nodes.at(2).s = -complex_t(0.004,0.002);
        net->grids.at(2).nodes.at(1).s = -complex_t(0.002,0.001);
        net->grids.at(2).nodes.at(2).s = -complex_t(0.005, 0.004);
        for (GridSolver* solverBFS : solvers) {
            solverBFS->solve();
            
        }
        
        //Load Single grid
        std::ifstream fileSG("/Users/simonhansson/U3/Kandidat01/examples/test_networks/test_network_single_grid.txt");
        CHECK_FALSE(fileSG.fail());
        NetworkLoader loaderSG(fileSG);
        std::unique_ptr<Network> netSG = loaderSG.loadNetwork();
        std::vector<GridSolver*> solversSG;
        for (Grid& grid : netSG->grids) {
            solversSG.push_back(new GaussSeidelSolver(&grid));
        }

        // 2 l (0.004, 0.002)
        // 1 l (0.002, 0.001)
        // 2 l (0.005, 0.004)
        netSG->grids.at(0).nodes.at(7).s = -complex_t(0.004,0.002);
        netSG->grids.at(0).nodes.at(5).s = -complex_t(0.002,0.001);
        netSG->grids.at(0).nodes.at(6).s = -complex_t(0.005, 0.004);

        for (GridSolver* solver : solversSG) {
            solver->solve();
        }

        std::cout << "net: " << std::endl;
        for (const Grid& grid : net->grids) {
            for (const GridNode& node : grid.nodes) {
                std::cout << node.v.real() << "," << node.v.imag() << "  " << node.s.real() << "," << node.s.imag() << std::endl;
            }
        }

        std::cout << "SG: " << std::endl;
        for (const Grid& grid : netSG->grids) {
            for (const GridNode& node : grid.nodes) {
                std::cout << node.v.real() << "," << node.v.imag() << "  " << node.s.real() << "," << node.s.imag() << std::endl;
            }
        }
    
        //Compare the result
        CHECK_THAT(netSG->grids[0].nodes[1].v.real(), Catch::Matchers::WithinAbs(net->grids[0].nodes[1].v.real(), 0.000001));
        CHECK_THAT(netSG->grids[0].nodes[2].v.real(), Catch::Matchers::WithinAbs(net->grids[0].nodes[2].v.real(), 0.000001));
        CHECK_THAT(netSG->grids[0].nodes[3].v.real(), Catch::Matchers::WithinAbs(net->grids[0].nodes[3].v.real(), 0.000001));
        CHECK_THAT(netSG->grids[0].nodes[4].v.real(), Catch::Matchers::WithinAbs(net->grids[1].nodes[1].v.real(), 0.000001));
        CHECK_THAT(netSG->grids[0].nodes[5].v.real(), Catch::Matchers::WithinAbs(net->grids[2].nodes[2].v.real(), 0.000001));
        CHECK_THAT(netSG->grids[0].nodes[6].v.real(), Catch::Matchers::WithinAbs(net->grids[2].nodes[2].v.real(), 0.000001));
        CHECK_THAT(netSG->grids[0].nodes[7].v.real(), Catch::Matchers::WithinAbs(net->grids[1].nodes[2].v.real(), 0.000001));
        
    }
}  



   