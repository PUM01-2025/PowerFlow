#include "powerflow/NetworkLoader.hpp"
#include "powerflow/GaussSeidelSolver.hpp"
#include "powerflow/BackwardForwardSweepSolver.hpp"
#include "powerflow/PowerFlowSolver.hpp"

#include <iostream>
#include <fstream>


//int main(int argc, char* argv[])
//{
//    std::ifstream file("C:\\Users\\melvi\\Kandidat01\\examples\\example_network.txt");
//    NetworkLoader loader(file);
//    std::shared_ptr<Network> net = loader.loadNetwork();
//    PowerFlowSolver pfs(net);
//    // pfs.solve(); SKICKA IN U HÄR! (Ska inte vara i datafilen!!!!)
//
//    for (const Grid& grid : net->grids) {
//        for (const GridNode& node : grid.nodes) {
//            std::cout << node.v.real() << "," << node.v.imag() << "  " << node.s.real() << "," << node.s.imag() << std::endl;
//        }
//    }
//}

int main(int argc, char* argv[])
{
    std::ifstream file("C:\\Users\\melvi\\Kandidat01\\examples\\example_network.txt");
    NetworkLoader loader(file);
    std::unique_ptr<Network> net = loader.loadNetwork();

    std::cout << "Loaded" << std::endl;

    std::vector<GridSolver*> solvers;
    for (Grid& grid : net->grids) {
        solvers.push_back(new BackwardForwardSweepSolver(&grid));
    }

    for (int i = 0; i < 100; ++i) {
        for (GridSolver* solver : solvers) {
            solver->solve();

            // Uppdatera connections (låtsaskablar med 0 impedans).
            for (GridConnection& connection : net->connections) {
                // OBS teckenbyte för s.
                net->grids[connection.slack_grid].nodes[connection.slack_node].s = -net->grids[connection.pq_grid].nodes[connection.pq_node].s;
                net->grids[connection.pq_grid].nodes[connection.pq_node].v = net->grids[connection.slack_grid].nodes[connection.slack_node].v;
            }
        }
    }

    for (const Grid& grid : net->grids) {
        for (const GridNode& node : grid.nodes) {
            std::cout << node.v.real() << "," << node.v.imag() << "  " << node.s.real() << "," << node.s.imag() << std::endl;
        }
    }

    return 0;
}
