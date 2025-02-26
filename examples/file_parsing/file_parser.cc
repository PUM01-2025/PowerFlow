#include <fstream>
#include <tuple>
#include <vector>
#include "../include/Eigen/Dense"
#include "../include/Eigen/Sparse"


using Eigen::SparseMatrix;

enum class NODE_T
{
    GENERATOR,
    JUNCTION,
    LOAD
};

template <typename info_T>
using node_info = std::unordered_map<unsigned int, info_T>;
using val_t = std::complex<double>;
using VectorXcd = Eigen::Matrix<val_t, Eigen::Dynamic, 1>;
using indx = unsigned int;

void load_grid_from_file(std::string filename)
{
    unsigned int size{};
    std::vector<std::tuple<indx,val_t,val_t>> nodes{};
    std::ifstream file{filename};
    
    // Parse Nodes
    
    indx node_idx{};
    val_t node_v{}, node_p{};
    
    file >> node_idx >> node_v >> node_p;
    // We need to know the size so this is unfortunate extra stuff :/
    while (node_idx != 1){
        nodes.push_back(std::make_tuple(node_idx, node_v, node_p));

    }

    size = nodes.size();

    VectorXcd nodes_whole_v{size};
    VectorXcd nodes_whole_p{size};

    for ( auto [x, v, p] : nodes){
        nodes_whole_v(x,x) = v;
        nodes_whole_v(x,x) = p;
    }

}

