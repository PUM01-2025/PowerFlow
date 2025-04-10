#include "powerflow/NetworkLoader.hpp"
#include "powerflow/GaussSeidelSolver.hpp"
#include "powerflow/BackwardForwardSweepSolver.hpp"
#include "powerflow/PowerFlowSolver.hpp"

#include <iostream>
#include <fstream>


int main(int argc, char* argv[])
{
    std::ifstream file("../examples/example_network_single_grid.txt");
    NetworkLoader loader(file);
    std::shared_ptr<Network> net = loader.loadNetwork();
    for (const Grid& grid : net->grids) {
        for (const GridNode& node : grid.nodes) {
            std::cout << node.v.real() << "," << node.v.imag() << "  " << node.s.real() << "," << node.s.imag() << std::endl;
        }
    }

    PowerFlowSolver pfs(net);
    std::vector<complex_t> P = {
        {0.002, 0.001},
        {0.005, 0.004},
        {0.004, 0.002}
    };
    std::vector<complex_t> U = pfs.solve(P);

    for (const Grid& grid : net->grids) {
        for (const GridNode& node : grid.nodes) {
            std::cout << node.v.real() << "," << node.v.imag() << "  " << node.s.real() << "," << node.s.imag() << std::endl;
        }
    }
}

//int main(int argc, char* argv[])
//{
//    std::ifstream file("C:\\Users\\melvi\\Kandidat01\\examples\\example_network.txt");
//    NetworkLoader loader(file);
//    std::unique_ptr<Network> net = loader.loadNetwork();
//
//    std::cout << "Loaded" << std::endl;
//
//    // 2 l (0.004, 0.002)
//    net->grids.at(1).nodes.at(2).s = -complex_t(0.004,0.002);
//    // 1 l (0.002, 0.001)
//    // 2 l (0.005, 0.004)
//    net->grids.at(2).nodes.at(1).s = -complex_t(0.002,0.001);
//    net->grids.at(2).nodes.at(2).s = -complex_t(0.005, 0.004);
//
//    std::vector<GridSolver*> solvers;
//    for (Grid& grid : net->grids) {
//        solvers.push_back(new BackwardForwardSweepSolver(&grid));
//    }
//
//    
//
//    for (int i = 0; i < 100; ++i) {
//        for (GridSolver* solver : solvers) {
//            solver->solve();
//
//            // Uppdatera connections (l�tsaskablar med 0 impedans).
//            for (GridConnection& connection : net->connections) {
//                // OBS teckenbyte f�r s.
//                net->grids[connection.slackGrid].nodes[connection.slackNode].s = -net->grids[connection.pqGrid].nodes[connection.pqNode].s;
//                net->grids[connection.pqGrid].nodes[connection.pqNode].v = net->grids[connection.slackGrid].nodes[connection.slackNode].v;
//            }
//        }
//    }
//
//    for (const Grid& grid : net->grids) {
//        for (const GridNode& node : grid.nodes) {
//            std::cout << node.v.real() << "," << node.v.imag() << "  " << node.s.real() << "," << node.s.imag() << std::endl;
//        }
//    }
//
//    return 0;
//}
