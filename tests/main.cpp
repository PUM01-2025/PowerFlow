#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "powerflow/NetworkLoader.hpp"
#include "powerflow/solvers/BackwardForwardSweepSolver.hpp"
#include "powerflow/solvers/GaussSeidelSolver.hpp"
#include "powerflow/solvers/ZBusJacobiSolver.hpp"
#include "powerflow/PowerFlowSolver.hpp"
#include "powerflow/GridAnalyzer.hpp"
#include "powerflow/logger/CppLogger.hpp"

#include <fstream>
#include <string>
#include <filesystem>

std::string localPath = "";

//CHECK_FALSE(file.fail());  checks that the file can be opened correctly (in most cases it is the wrong filepath)
bool test_input_error_message(std::string errorMessage, std::string filePath){
    std::ifstream file(filePath);
    CHECK_FALSE(file.fail()); 
    NetworkLoader loader(file);
    REQUIRE_THROWS_WITH(loader.loadNetwork(), Catch::Matchers::Contains(errorMessage)); //should fail and halt so true is not returned
    return true;
}

TEST_CASE("Networkloader input", "[!throws]" ) {
    std::cout << "LOCALPATH: " << localPath << std::endl;
    REQUIRE(test_input_error_message("Invalid S base", localPath + "examples/test_networks/invalid_base_S.txt"));
    REQUIRE(test_input_error_message("Invalid V base", localPath + "examples/test_networks/invalid_base_V.txt"));
    REQUIRE(test_input_error_message("Invalid command", localPath + "examples/test_networks/invalid_command.txt"));
    REQUIRE(test_input_error_message("Invalid node index", localPath + "examples/test_networks/invalid_node_index.txt"));
    REQUIRE(test_input_error_message("Empty grid", localPath +  "examples/test_networks/empty_grid.txt"));
    REQUIRE(test_input_error_message("Invalid edge parent index", localPath + "examples/test_networks/invalid_edge_parent_index.txt"));
    REQUIRE(test_input_error_message("Invalid edge child index", localPath + "examples/test_networks/invalid_edge_child_index.txt"));
    REQUIRE(test_input_error_message("Invalid edge impedance", localPath + "examples/test_networks/invalid_edge_impedance_index.txt"));
    REQUIRE(test_input_error_message("Invalid grid index", localPath + "examples/test_networks/invalid_slack_grid_index.txt"));
    REQUIRE(test_input_error_message("Invalid LOAD_IMPLICIT node index", localPath + "examples/test_networks/invalid_slack_node_index.txt"));
    REQUIRE(test_input_error_message("Invalid grid index", localPath + "examples/test_networks/invalid_PQ_grid_index.txt"));
    REQUIRE(test_input_error_message("Invalid SLACK_IMPLICIT node index", localPath + "examples/test_networks/invalid_PQ_node_index.txt"));
    REQUIRE(test_input_error_message("Invalid node type", localPath + "examples/test_networks/invalid_node_type.txt"));
}

void validatePFSThrow(const std::string& filePath)
{
    std::ifstream file(filePath);
    CHECK_FALSE(file.fail());

    NetworkLoader loader(file);
    std::unique_ptr<Network> net = loader.loadNetwork();
    CppLogger logger(std::cout);
    SolverSettings settings{};
    PowerFlowSolver pfs(std::move(net), settings, &logger);
}

TEST_CASE("Network validation", "[!throws]") {
    REQUIRE_THROWS_WITH(validatePFSThrow(localPath + "examples/test_networks/invalid_node_type_in_connection.txt"),
        Catch::Matchers::Contains("Invalid node type in connection 0"));

    REQUIRE_THROWS_WITH(validatePFSThrow(localPath + "examples/test_networks/invalid_node_type_in_connection_2.txt"),
        Catch::Matchers::Contains("Invalid node type in connection 1"));

    REQUIRE_THROWS_WITH(validatePFSThrow(localPath + "examples/test_networks/missing_connections.txt"),
        Catch::Matchers::Contains("Grid 0 not properly connected to the rest of the network"));

    REQUIRE_THROWS_WITH(validatePFSThrow(localPath + "examples/test_networks/double_edge.txt"),
        Catch::Matchers::Contains("More than one edge detected between node 0 and node 2 in grid 2"));

    REQUIRE_THROWS_WITH(validatePFSThrow(localPath + "examples/test_networks/double_connection.txt"),
        Catch::Matchers::Contains("Grid 0 not properly connected to the rest of the network"));

    REQUIRE_THROWS_WITH(validatePFSThrow(localPath + "examples/test_networks/missing_slack.txt"),
        Catch::Matchers::Contains("Missing slack node in grid 3"));

    REQUIRE_THROWS_WITH(validatePFSThrow(localPath + "examples/test_networks/inner_connection.txt"),
        Catch::Matchers::Contains("Connection in the same grid 2 not allowed"));

    REQUIRE_THROWS_WITH(validatePFSThrow(localPath + "examples/test_networks/edge_same_node.txt"),
        Catch::Matchers::Contains("Invalid edge 2 that connects to the same node in grid 1"));

    REQUIRE_THROWS_WITH(validatePFSThrow(localPath + "examples/test_networks/zero_impedance.txt"),
        Catch::Matchers::Contains("Invalid zero impedance in edge 1 in grid 1"));
    
    // Disjoint grid.
    REQUIRE_THROWS_WITH([]() {
        std::unique_ptr<Network> net = std::make_unique<Network>();
        Grid grid;
        GridNode node1;
        GridNode node2;
        node1.type = SLACK;
        node2.type = LOAD;
        grid.nodes.push_back(node1);
        grid.nodes.push_back(node2);
        net->grids.push_back(grid);
        CppLogger logger(std::cout);
        SolverSettings settings{};
        PowerFlowSolver pfs(std::move(net), settings, &logger);
    }(), Catch::Matchers::Contains("Grid 0 consists of multiple disjoint graphs"));


}


TEST_CASE("Compare output of BFS and GS and zbus jacobi","[validation]"){

    SECTION("test_network.txt"){
        //Load BFS
        std::ifstream fileBFS(localPath + "examples/test_networks/test_network.txt"); //Ladda in testfil
        CHECK_FALSE(fileBFS.fail());                                            //Kommer ge en varning att om det blir fel i filinläsningen
        NetworkLoader loaderBFS(fileBFS);                                       //Skapa en loader
        std::unique_ptr<Network> netBFS = loaderBFS.loadNetwork();              //Ladda in nätveket
        std::vector<GridSolver*> solversBFS;                                    //Vector att spara läsarna i
        CppLogger logger(std::cout);
        for (Grid& grid : netBFS->grids) {                                      //Loopa igenom nätet och lätt till en lösare för varje subnät
            solversBFS.push_back(new BackwardForwardSweepSolver(&grid, &logger, 10000, 1e-10));
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
        std::ifstream file(localPath + "examples/test_networks/test_network.txt");
        CHECK_FALSE(file.fail());
        NetworkLoader loader(file);
        std::unique_ptr<Network> net = loader.loadNetwork();
        std::vector<GridSolver*> solvers;
        for (Grid& grid : net->grids) {
            solvers.push_back(new GaussSeidelSolver(&grid, &logger, 100000, 1e-10));
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
                //Kollar att GS svaret är inom 1e-10 av BFS svaret med hjälp av en catch2 matcher
                CHECK_THAT(net->grids[i].nodes[j].v.real(), Catch::Matchers::WithinAbs(netBFS->grids[i].nodes[j].v.real(), 1e-10));
                CHECK_THAT(net->grids[i].nodes[j].v.imag(), Catch::Matchers::WithinAbs(netBFS->grids[i].nodes[j].v.imag(), 1e-10));
                CHECK_THAT(net->grids[i].nodes[j].s.real(), Catch::Matchers::WithinAbs(netBFS->grids[i].nodes[j].s.real(), 1e-10));
                CHECK_THAT(net->grids[i].nodes[j].s.imag(), Catch::Matchers::WithinAbs(netBFS->grids[i].nodes[j].s.imag(), 1e-10));
            }
        }
    }
}

TEST_CASE("Compare GS and ZBus Jacobi", "[validation]") {
    CppLogger logger(std::cout);

    // Load Gauss-Seidel
    std::ifstream file(localPath + "examples/test_networks/test_network_cycle.txt");
    CHECK_FALSE(file.fail());
    NetworkLoader loader(file);
    std::unique_ptr<Network> net = loader.loadNetwork();
    std::vector<GridSolver*> solvers;
    for (Grid& grid : net->grids) {
        solvers.push_back(new GaussSeidelSolver(&grid, &logger, 100000, 1e-10));
    }
    net->grids.at(0).nodes.at(7).s = -complex_t(0.004, 0.002);
    net->grids.at(0).nodes.at(5).s = -complex_t(0.002, 0.001);
    net->grids.at(0).nodes.at(6).s = -complex_t(0.005, 0.004);
    for (GridSolver* solver : solvers) {
        solver->solve();
    }

    //Load zbus jacobi //Samma som ovan men för ZBus Jacobi
    std::ifstream file2(localPath + "examples/test_networks/test_network_cycle.txt");
    CHECK_FALSE(file2.fail());
    NetworkLoader loader2(file2);
    std::unique_ptr<Network> net2 = loader2.loadNetwork();
    std::vector<GridSolver*> solvers2;
    for (Grid& grid : net2->grids) {
        solvers2.push_back(new ZBusJacobiSolver(&grid, &logger, 100000, 1e-10));
    }
    net2->grids.at(0).nodes.at(7).s = -complex_t(0.004, 0.002);
    net2->grids.at(0).nodes.at(5).s = -complex_t(0.002, 0.001);
    net2->grids.at(0).nodes.at(6).s = -complex_t(0.005, 0.004);
    for (GridSolver* solver : solvers2) {
        solver->solve();
    }

    CHECK_THAT(net->grids[0].nodes[7].v.real(), Catch::Matchers::WithinAbs(net2->grids[0].nodes[7].v.real(), 1e-10));
    CHECK_THAT(net->grids[0].nodes[5].v.real(), Catch::Matchers::WithinAbs(net2->grids[0].nodes[5].v.real(), 1e-10));
    CHECK_THAT(net->grids[0].nodes[6].v.real(), Catch::Matchers::WithinAbs(net2->grids[0].nodes[6].v.real(), 1e-10));

    CHECK_THAT(net->grids[0].nodes[7].v.imag(), Catch::Matchers::WithinAbs(net2->grids[0].nodes[7].v.imag(), 1e-10));
    CHECK_THAT(net->grids[0].nodes[5].v.imag(), Catch::Matchers::WithinAbs(net2->grids[0].nodes[5].v.imag(), 1e-10));
    CHECK_THAT(net->grids[0].nodes[6].v.imag(), Catch::Matchers::WithinAbs(net2->grids[0].nodes[6].v.imag(), 1e-10));
}

TEST_CASE("Compare treestructure", "[validation]") {
    std::ifstream file(localPath + "examples/test_networks/test_network.txt");
    CHECK_FALSE(file.fail());
    NetworkLoader loader(file);
    std::shared_ptr<Network> net = loader.loadNetwork();
    CppLogger logger(std::cout);
    SolverSettings settings{};
    PowerFlowSolver pfs(net, settings, &logger);
    std::vector<complex_t> P = {
        {0.002, 0.001},
        {0.005, 0.004},
        {0.004, 0.002}
    };  
    std::vector<complex_t> V = { {1,0 } };
    pfs.solve(P, V);

    std::ifstream fileSingle(localPath + "examples/test_networks/test_network_single_grid.txt");
    CHECK_FALSE(fileSingle.fail()); 
    NetworkLoader loaderSingle(fileSingle);
    std::shared_ptr<Network> netSingle = loaderSingle.loadNetwork();
    PowerFlowSolver pfsSingle(netSingle, settings, &logger);
    std::vector<complex_t> PSingle = {
        {0.005, 0.004},
        {0.004, 0.002},
        {0.002, 0.001}
    };
    std::vector<complex_t> VSingle = { {1, 0} };
    pfsSingle.solve(PSingle, VSingle);

    CHECK_THAT(netSingle->grids[0].nodes[1].v.real(), Catch::Matchers::WithinAbs(net->grids[0].nodes[1].v.real(), 1e-10));
    CHECK_THAT(netSingle->grids[0].nodes[2].v.real(), Catch::Matchers::WithinAbs(net->grids[0].nodes[2].v.real(), 1e-10));
    CHECK_THAT(netSingle->grids[0].nodes[3].v.real(), Catch::Matchers::WithinAbs(net->grids[0].nodes[3].v.real(), 1e-10));
    CHECK_THAT(netSingle->grids[0].nodes[4].v.real(), Catch::Matchers::WithinAbs(net->grids[1].nodes[1].v.real(), 1e-10));
    CHECK_THAT(netSingle->grids[0].nodes[5].v.real(), Catch::Matchers::WithinAbs(net->grids[2].nodes[1].v.real(), 1e-10));
    CHECK_THAT(netSingle->grids[0].nodes[6].v.real(), Catch::Matchers::WithinAbs(net->grids[2].nodes[2].v.real(), 1e-10));
    CHECK_THAT(netSingle->grids[0].nodes[7].v.real(), Catch::Matchers::WithinAbs(net->grids[1].nodes[2].v.real(), 1e-10));

    CHECK_THAT(netSingle->grids[0].nodes[1].v.imag(), Catch::Matchers::WithinAbs(net->grids[0].nodes[1].v.imag(), 1e-10));
    CHECK_THAT(netSingle->grids[0].nodes[2].v.imag(), Catch::Matchers::WithinAbs(net->grids[0].nodes[2].v.imag(), 1e-10));
    CHECK_THAT(netSingle->grids[0].nodes[3].v.imag(), Catch::Matchers::WithinAbs(net->grids[0].nodes[3].v.imag(), 1e-10));
    CHECK_THAT(netSingle->grids[0].nodes[4].v.imag(), Catch::Matchers::WithinAbs(net->grids[1].nodes[1].v.imag(), 1e-10));
    CHECK_THAT(netSingle->grids[0].nodes[5].v.imag(), Catch::Matchers::WithinAbs(net->grids[2].nodes[1].v.imag(), 1e-10));
    CHECK_THAT(netSingle->grids[0].nodes[6].v.imag(), Catch::Matchers::WithinAbs(net->grids[2].nodes[2].v.imag(), 1e-10));
    CHECK_THAT(netSingle->grids[0].nodes[7].v.imag(), Catch::Matchers::WithinAbs(net->grids[1].nodes[2].v.imag(), 1e-10));

    CHECK_THAT(netSingle->grids[0].nodes[1].s.real(), Catch::Matchers::WithinAbs(net->grids[0].nodes[1].s.real(), 1e-10));
    CHECK_THAT(netSingle->grids[0].nodes[2].s.real(), Catch::Matchers::WithinAbs(net->grids[0].nodes[2].s.real(), 1e-10));
    CHECK_THAT(netSingle->grids[0].nodes[3].s.real(), Catch::Matchers::WithinAbs(net->grids[0].nodes[3].s.real(), 1e-10));
    CHECK_THAT(netSingle->grids[0].nodes[4].s.real(), Catch::Matchers::WithinAbs(net->grids[1].nodes[1].s.real(), 1e-10));
    CHECK_THAT(netSingle->grids[0].nodes[5].s.real(), Catch::Matchers::WithinAbs(net->grids[2].nodes[1].s.real(), 1e-10));
    CHECK_THAT(netSingle->grids[0].nodes[6].s.real(), Catch::Matchers::WithinAbs(net->grids[2].nodes[2].s.real(), 1e-10));
    CHECK_THAT(netSingle->grids[0].nodes[7].s.real(), Catch::Matchers::WithinAbs(net->grids[1].nodes[2].s.real(), 1e-10));

    CHECK_THAT(netSingle->grids[0].nodes[1].s.imag(), Catch::Matchers::WithinAbs(net->grids[0].nodes[1].s.imag(), 1e-10));
    CHECK_THAT(netSingle->grids[0].nodes[2].s.imag(), Catch::Matchers::WithinAbs(net->grids[0].nodes[2].s.imag(), 1e-10));
    CHECK_THAT(netSingle->grids[0].nodes[3].s.imag(), Catch::Matchers::WithinAbs(net->grids[0].nodes[3].s.imag(), 1e-10));
    CHECK_THAT(netSingle->grids[0].nodes[4].s.imag(), Catch::Matchers::WithinAbs(net->grids[1].nodes[1].s.imag(), 1e-10));
    CHECK_THAT(netSingle->grids[0].nodes[5].s.imag(), Catch::Matchers::WithinAbs(net->grids[2].nodes[1].s.imag(), 1e-10));
    CHECK_THAT(netSingle->grids[0].nodes[6].s.imag(), Catch::Matchers::WithinAbs(net->grids[2].nodes[2].s.imag(), 1e-10));
    CHECK_THAT(netSingle->grids[0].nodes[7].s.imag(), Catch::Matchers::WithinAbs(net->grids[1].nodes[2].s.imag(), 1e-10));
    
}

//AUTHOR: Brenner   
TEST_CASE("Choose solver", "[validation]"){
    SECTION("GridAnalyzer"){

        std::ifstream tree_file( localPath + "examples/test_networks/test_network.txt");                        //Ladda in exempelnätverk med trädstruktur
        CHECK_FALSE(tree_file.fail());                                              //Säkerställ att filen kunde laddas in
        NetworkLoader tree_loader(tree_file);                                       //Skapa en loader
        std::unique_ptr<Network> tree_network = tree_loader.loadNetwork();          //Spara som nätverk
        GridAnalyzer analyzer;
        for(unsigned long i = 0; i < tree_network->grids.size(); i++){
            
            REQUIRE(analyzer.determineSolver(tree_network->grids[i]) == BACKWARDFOWARDSWEEP); //Ingen grid får ha en cykel
        }
    
        std::ifstream cycle_file(localPath + "examples/test_networks/test_network_cycle.txt");                 //Ladda in exempelnätverk med cykel
        CHECK_FALSE(cycle_file.fail());                                             //Säkerställ att filen kunde laddas in
        NetworkLoader cycle_loader(cycle_file);                                     //Skapa en loader
        std::unique_ptr<Network> cycle_network = cycle_loader.loadNetwork();        //Spara som nätverk
        bool containsCycle = false;                                                 //Blir sann om det finns åtminstone en cykel
        for(unsigned long i = 0; i < cycle_network->grids.size(); i++){
            if(analyzer.determineSolver(cycle_network->grids[i]) != BACKWARDFOWARDSWEEP){
                containsCycle = true;
            }
        }
        REQUIRE(containsCycle);

        std::ifstream slack_file(localPath + "examples/test_networks/multiple_slack_nodes.txt");
        CHECK_FALSE(slack_file.fail());
        NetworkLoader slack_loader(slack_file);
        std::unique_ptr<Network> slack_network = slack_loader.loadNetwork();
        SolverType solverTypeSlack = analyzer.determineSolver(slack_network->grids[2]);
        REQUIRE(solverTypeSlack != ZBUSJACOBI);
        REQUIRE(solverTypeSlack != BACKWARDFOWARDSWEEP);
    }
}
