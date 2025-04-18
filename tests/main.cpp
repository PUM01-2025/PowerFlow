#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "powerflow/NetworkLoader.hpp"
#include "powerflow/BackwardForwardSweepSolver.hpp"
#include "powerflow/GaussSeidelSolver.hpp"
#include "powerflow/PowerFlowSolver.hpp"
#include "powerflow/NetworkAnalyzer.hpp"

#include <fstream>
#include <string>

//std::string localPath = "/Users/simonhansson/U3/Kandidat01/";
std::string localPath = "/home/runner/work/Kandidat01/Kandidat01/";
//CHECK_FALSE(file.fail());  checks that the file can be opened correctly (in most cases it is the wrong filepath)

bool test_input_error_message(std::string errorMessage, std::string filePath){
    std::ifstream file(filePath);
    CHECK_FALSE(file.fail()); 
    NetworkLoader loader(file);
    REQUIRE_THROWS_WITH(loader.loadNetwork(), Catch::Matchers::Contains(errorMessage)); //should fail and halt so true is not returned
    return true;
}

TEST_CASE("Networkloader input", "[!throws]" ) {
    REQUIRE(test_input_error_message("Invalid S base", localPath + "examples/test_networks/invalid_base_S.txt"));
    REQUIRE(test_input_error_message("Invalid V base", localPath + "examples/test_networks/invalid_base_V.txt"));
    REQUIRE(test_input_error_message("Invalid command", localPath + "examples/test_networks/invalid_command.txt"));
    REQUIRE(test_input_error_message("Invalid node index", localPath + "examples/test_networks/invalid_node_index.txt"));
    REQUIRE(test_input_error_message("Empty grid", localPath +  "examples/test_networks/empty_grid.txt"));
    REQUIRE(test_input_error_message("Invalid edge parent index", localPath + "examples/test_networks/invalid_edge_parent_index.txt"));
    REQUIRE(test_input_error_message("Invalid edge child index", localPath + "examples/test_networks/invalid_edge_child_index.txt"));
    REQUIRE(test_input_error_message("Invalid edge impedance", localPath + "examples/test_networks/invalid_edge_impedance_index.txt"));
    REQUIRE(test_input_error_message("Invalid slack grid index", localPath + "examples/test_networks/invalid_slack_grid_index.txt"));
    REQUIRE(test_input_error_message("Invalid slack node index", localPath + "examples/test_networks/invalid_slack_node_index.txt"));
    REQUIRE(test_input_error_message("Invalid PQ grid index", localPath + "examples/test_networks/invalid_PQ_grid_index.txt"));
    REQUIRE(test_input_error_message("Invalid PQ node index", localPath + "examples/test_networks/invalid_PQ_node_index.txt"));

    //This does not work, check the test file to see what error messeges you get instead
    std::ifstream file(localPath + "examples/test_networks/invalid_end_of_list.txt");
    CHECK_FALSE(file.fail()); 
    NetworkLoader loader(file);
    //REQUIRE_THROWS_WITH(loader.loadNetwork(), Catch::Matchers::Contains("Missing end-of-list indicator"));
}


TEST_CASE("Compare output of BFS and GS","[validation]"){

    SECTION("example_network.txt"){
        //Load BFS
        std::ifstream fileBFS(localPath + "examples/example_network.txt"); //Ladda in testfil
        CHECK_FALSE(fileBFS.fail());                                            //Kommer ge en varning att om det blir fel i filinläsningen
        NetworkLoader loaderBFS(fileBFS);                                       //Skapa en loader
        std::unique_ptr<Network> netBFS = loaderBFS.loadNetwork();              //Ladda in nätveket
        std::vector<GridSolver*> solversBFS;                                    //Vector att spara läsarna i
        for (Grid& grid : netBFS->grids) {                                      //Loopa igenom nätet och lätt till en lösare för varje subnät
            solversBFS.push_back(new BackwardForwardSweepSolver(&grid));
        }

        //Ladda in effektbelastningar
        netBFS->grids.at(1).nodes.at(2).s = -complex_t(0.004,0.002);
        netBFS->grids.at(2).nodes.at(1).s = -complex_t(0.002,0.001);
        netBFS->grids.at(2).nodes.at(2).s = -complex_t(0.005, 0.004);

        //Loopa över alla lösare och kör dem
        for (GridSolver* solverBFS : solversBFS) {
            solverBFS->solve();
        }
        
        //Load GS //Samma som ovan men för GaussSeidel
        std::ifstream file(localPath + "examples/example_network.txt");
        CHECK_FALSE(file.fail());
        NetworkLoader loader(file);
        std::unique_ptr<Network> net = loader.loadNetwork();
        std::vector<GridSolver*> solvers;
        for (Grid& grid : net->grids) {
            solvers.push_back(new GaussSeidelSolver(&grid));
        }
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
    std::ifstream file(localPath + "examples/test_networks/test_network.txt");
    CHECK_FALSE(file.fail()); 
    NetworkLoader loader(file);
    std::shared_ptr<Network> net = loader.loadNetwork();
    PowerFlowSolver pfs(net);
    std::vector<complex_t> P = {
        {0.002, 0.001},
        {0.005, 0.004},
        {0.004, 0.002}
    };  
    std::vector<complex_t> U = pfs.solve(P);

    std::ifstream fileSingle(localPath + "examples/test_networks/test_network_single_grid.txt");
    CHECK_FALSE(fileSingle.fail()); 
    NetworkLoader loaderSingle(fileSingle);
    std::shared_ptr<Network> netSingle = loaderSingle.loadNetwork();
    PowerFlowSolver pfsSingle(netSingle);
    std::vector<complex_t> PSingle = {
        {0.005, 0.004},
        {0.004, 0.002},
        {0.002, 0.001}
    };
    std::vector<complex_t> USingle = pfsSingle.solve(PSingle);

    CHECK_THAT(netSingle->grids[0].nodes[1].v.real(), Catch::Matchers::WithinAbs(net->grids[0].nodes[1].v.real(), 0.000001));
    CHECK_THAT(netSingle->grids[0].nodes[2].v.real(), Catch::Matchers::WithinAbs(net->grids[0].nodes[2].v.real(), 0.000001));
    CHECK_THAT(netSingle->grids[0].nodes[3].v.real(), Catch::Matchers::WithinAbs(net->grids[0].nodes[3].v.real(), 0.000001));
    CHECK_THAT(netSingle->grids[0].nodes[4].v.real(), Catch::Matchers::WithinAbs(net->grids[1].nodes[1].v.real(), 0.000001));
    CHECK_THAT(netSingle->grids[0].nodes[5].v.real(), Catch::Matchers::WithinAbs(net->grids[2].nodes[1].v.real(), 0.000001));
    CHECK_THAT(netSingle->grids[0].nodes[6].v.real(), Catch::Matchers::WithinAbs(net->grids[2].nodes[2].v.real(), 0.000001));
    CHECK_THAT(netSingle->grids[0].nodes[7].v.real(), Catch::Matchers::WithinAbs(net->grids[1].nodes[2].v.real(), 0.000001));

    CHECK_THAT(netSingle->grids[0].nodes[1].v.imag(), Catch::Matchers::WithinAbs(net->grids[0].nodes[1].v.imag(), 0.000001));
    CHECK_THAT(netSingle->grids[0].nodes[2].v.imag(), Catch::Matchers::WithinAbs(net->grids[0].nodes[2].v.imag(), 0.000001));
    CHECK_THAT(netSingle->grids[0].nodes[3].v.imag(), Catch::Matchers::WithinAbs(net->grids[0].nodes[3].v.imag(), 0.000001));
    CHECK_THAT(netSingle->grids[0].nodes[4].v.imag(), Catch::Matchers::WithinAbs(net->grids[1].nodes[1].v.imag(), 0.000001));
    CHECK_THAT(netSingle->grids[0].nodes[5].v.imag(), Catch::Matchers::WithinAbs(net->grids[2].nodes[1].v.imag(), 0.000001));
    CHECK_THAT(netSingle->grids[0].nodes[6].v.imag(), Catch::Matchers::WithinAbs(net->grids[2].nodes[2].v.imag(), 0.000001));
    CHECK_THAT(netSingle->grids[0].nodes[7].v.imag(), Catch::Matchers::WithinAbs(net->grids[1].nodes[2].v.imag(), 0.000001));

    CHECK_THAT(netSingle->grids[0].nodes[1].s.real(), Catch::Matchers::WithinAbs(net->grids[0].nodes[1].s.real(), 0.000001));
    CHECK_THAT(netSingle->grids[0].nodes[2].s.real(), Catch::Matchers::WithinAbs(net->grids[0].nodes[2].s.real(), 0.000001));
    CHECK_THAT(netSingle->grids[0].nodes[3].s.real(), Catch::Matchers::WithinAbs(net->grids[0].nodes[3].s.real(), 0.000001));
    CHECK_THAT(netSingle->grids[0].nodes[4].s.real(), Catch::Matchers::WithinAbs(net->grids[1].nodes[1].s.real(), 0.000001));
    CHECK_THAT(netSingle->grids[0].nodes[5].s.real(), Catch::Matchers::WithinAbs(net->grids[2].nodes[1].s.real(), 0.000001));
    CHECK_THAT(netSingle->grids[0].nodes[6].s.real(), Catch::Matchers::WithinAbs(net->grids[2].nodes[2].s.real(), 0.000001));
    CHECK_THAT(netSingle->grids[0].nodes[7].s.real(), Catch::Matchers::WithinAbs(net->grids[1].nodes[2].s.real(), 0.000001));

    CHECK_THAT(netSingle->grids[0].nodes[1].s.imag(), Catch::Matchers::WithinAbs(net->grids[0].nodes[1].s.imag(), 0.000001));
    CHECK_THAT(netSingle->grids[0].nodes[2].s.imag(), Catch::Matchers::WithinAbs(net->grids[0].nodes[2].s.imag(), 0.000001));
    CHECK_THAT(netSingle->grids[0].nodes[3].s.imag(), Catch::Matchers::WithinAbs(net->grids[0].nodes[3].s.imag(), 0.000001));
    CHECK_THAT(netSingle->grids[0].nodes[4].s.imag(), Catch::Matchers::WithinAbs(net->grids[1].nodes[1].s.imag(), 0.000001));
    CHECK_THAT(netSingle->grids[0].nodes[5].s.imag(), Catch::Matchers::WithinAbs(net->grids[2].nodes[1].s.imag(), 0.000001));
    CHECK_THAT(netSingle->grids[0].nodes[6].s.imag(), Catch::Matchers::WithinAbs(net->grids[2].nodes[2].s.imag(), 0.000001));
    CHECK_THAT(netSingle->grids[0].nodes[7].s.imag(), Catch::Matchers::WithinAbs(net->grids[1].nodes[2].s.imag(), 0.000001));
    
}

//AUTHOR: Brenner   
//    TEST_CASE("Choose solver", "[validation]"){
//     SECTION("NetworkAnalyzer"){

//     std::ifstream tree_file("/Users/simonhansson/U3/Kandidat01/examples/test_networks/test_network.txt");                        //Ladda in exempelnätverk med trädstruktur
//     CHECK_FALSE(tree_file.fail());                                              //Säkerställ att filen kunde laddas in
//     NetworkLoader tree_loader(tree_file);                                       //Skapa en loader
//     std::unique_ptr<Network> tree_network = tree_loader.loadNetwork();          //Spara som nätverk
    
//     for(int i = 0; tree_network->grids.size(); i++){
        
//         REQUIRE(determine_solver(tree_network->grids[i]) == BACKWARDFOWARDSWEEP); //Ingen grid får ha en cykel
//     }
   
//     std::ifstream cycle_file("/Users/simonhansson/U3/Kandidat01/examples/test_networks/test_network_cycle.txt");                 //Ladda in exempelnätverk med cykel
//     CHECK_FALSE(cycle_file.fail());                                             //Säkerställ att filen kunde laddas in
//     NetworkLoader cycle_loader(cycle_file);                                     //Skapa en loader
//     std::unique_ptr<Network> cycle_network = cycle_loader.loadNetwork();        //Spara som nätverk
//     bool containsCycle = false;                                                 //Blir sann om det finns åtminstone en cykel
//     for(int i = 0; tree_network->grids.size(); i++){
//         if(determine_solver(tree_network->grids[i]) == GAUSSSEIDEL){
//             containsCycle = true;
//         }
//     }
//     REQUIRE(containsCycle);
//     }
//}