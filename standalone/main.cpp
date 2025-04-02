#include "powerflow/NetworkLoader.hpp"
#include "powerflow/GaussSeidelSolver.hpp"

#include <iostream>
#include <fstream>


void printResults(const Network& network) {

}


int main(int argc, char* argv[])
{
    std::ifstream file("C:\\Users\\melvi\\Kandidat01\\examples\\example_network.txt");
    NetworkLoader loader(file);
    std::unique_ptr<Network> net = loader.loadNetwork();

    std::cout << "Loaded" << std::endl;

    std::vector<GaussSeidelSolver> solvers;
    for (Grid& grid : net->grids) {
        solvers.push_back(GaussSeidelSolver(&grid));
    }

    for (int i = 0; i < 100; ++i) {
        for (GaussSeidelSolver& solver : solvers) {
            solver.solve();

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
