#include <iostream>
#include <fstream>
#include <vector>

// Include PowerFlow headers.
#include "powerflow/NetworkLoader.hpp"
#include "powerflow/PowerFlowSolver.hpp"
#include "powerflow/logger/CppLogger.hpp"

int main(int argc, char* argv[])
{
    // Open the network file using an ifstream.
    std::ifstream file("example_network_single_grid.txt");

    if (!file)
    {
        std::cerr << "Could not open network file" << std::endl;
        return -1;
    }

    // Create a NetworkLoader object and provide it with the file input stream.
    // Call the loadNetwork() method to load the network file into a Network
    // struct. In case of errors in the network file, the method will throw a
    // NetworkLoaderError.
    //
    // NOTE: A NetworkLoader object is only meant to be used once. To load
    // additional network files, it is recommended to create new NetworkLoaders
    // for each network file to be loaded.
    // 
    // NOTE: Using NetworkLoader to create a Network struct is not mandatory.
    // A Network struct could be created by some other method or even manually.
    NetworkLoader loader(file);
    std::unique_ptr<Network> net = loader.loadNetwork();

    // Create a PowerFlowSolver object and provide it with the network and a
    // logger. In this case, the CppLogger class is used. You can also
    // implement your own logger if it is needed in your environment.
    // 
    // IMPORTANT: From here on, the PowerFlowSolver owns the Network struct.
    // The Network struct can (should) not be modified by any other code at
    // this point!
    CppLogger logger(std::cout);
    SolverSettings settings{}; // Create a default settings object.
    PowerFlowSolver pfs(std::move(net), settings, &logger);

    // Create S and V vectors for the LOAD and SLACK_EXTERNAL nodes.
    std::vector<complex_t> S = {
        {0.002, 0.001},
        {0.005, 0.004},
        {0.004, 0.002}
    };
    std::vector<complex_t> V = { {1, 0} };

    // Run the solver by calling PowerFlowSolver::solve.
    pfs.solve(S, V);

    // Get the resulting voltages at the LOAD nodes.
    std::vector<complex_t> loadVoltages = pfs.getLoadVoltages();

    // Print the calculated voltages to cout and exit.
    for (const complex_t v : loadVoltages)
    {
        std::cout << "(" << v.real() << ", " << v.imag() << ")" << std::endl;
    }
}
