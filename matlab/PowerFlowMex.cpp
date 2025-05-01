#include <unordered_map>
#include <fstream>
#include <memory>
#include <cstdint>

#include "powerflow/NetworkLoader.hpp"
#include "powerflow/PowerFlowSolver.hpp"
#include "powerflow/network.hpp"
#include "MatlabLogger.hpp"
#include "mexAdapter.hpp"
#include "mex.hpp"

class MexFunction : public matlab::mex::Function
{
    std::unordered_map<std::uint64_t, std::unique_ptr<PowerFlowSolver>> solvers;
    std::uint64_t handleCounter = 0;

    // Pointer to MATLAB engine
    std::shared_ptr<matlab::engine::MATLABEngine> matlabPtr = getEngine();

    MatlabLogger logger{getEngine(), LogLevel::DEBUG};

public:
    void operator()(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs)
    {
        if (inputs.size() < 1 || inputs[0].getType() != matlab::data::ArrayType::MATLAB_STRING)
        {
            throw std::invalid_argument("Missing first argument: command");
        }

        std::string command = inputs[0][0];

        if (command == "load")
        {
            loadNetwork(outputs, inputs);
        }
        else if (command == "solve")
        {
            solve(outputs, inputs);
        }
        else if (command == "getLoadVoltages")
        {
            getLoadVoltages(outputs, inputs);
        }
        else if (command == "getAllVoltages")
        {
            getAllVoltages(outputs, inputs);
        }
        else if (command == "getCurrents")
        {
            getCurrents(outputs, inputs);
        }
        else if (command == "getSlackPowers")
        {
            getSlackPowers(outputs, inputs);
        }
        else if (command == "unload")
        {
            unloadNetwork(outputs, inputs);
        }
        else
        {
            throw std::invalid_argument("Invalid command");
        }
    }

private:
    std::uint64_t getSolverHandle(matlab::mex::ArgumentList inputs)
    {
        if (inputs.size() < 2 || inputs[1].getType() != matlab::data::ArrayType::UINT64 || inputs[1].getNumberOfElements() != 1)
        {
            throw std::invalid_argument("Invalid or missing Network handle");
        }
        return inputs[1][0];
    }

    void loadNetwork(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs)
    {
        if (inputs.size() < 2 || inputs[1].getType() != matlab::data::ArrayType::MATLAB_STRING)
        {
            throw std::invalid_argument("Missing file path");
        }

        std::string filePath = inputs[1][0];
        std::ifstream file(filePath);

        if (!file)
        {
            throw std::runtime_error("Could not open Network file");
        }

        SolverSettings settings;

        // Load options struct.
        if (inputs.size() >= 3)
        {
            if (inputs[2].getType() != matlab::data::ArrayType::STRUCT || inputs[2].getNumberOfElements() != 1)
            {
                throw std::invalid_argument("Settings not a valid Matlab struct");
            }

            matlab::data::StructArray options = inputs[2];

            for (std::string fieldName : options.getFieldNames())
            {
                const matlab::data::Array field = options[0][fieldName];

                if (fieldName == "max_iterations_total")
                {
                    if (field.getType() != matlab::data::ArrayType::DOUBLE || field.getNumberOfElements() != 1)
                    {
                        throw std::invalid_argument("Invalid max_iterations_total");
                    }
                    settings.max_iterations_total = field[0];
                }
                else if (fieldName == "max_iterations_gauss")
                {
                    if (field.getType() != matlab::data::ArrayType::DOUBLE || field.getNumberOfElements() != 1)
                    {
                        throw std::invalid_argument("Invalid max_iterations_gauss");
                    }
                    settings.max_iterations_gauss = field[0];
                }
                else if (fieldName == "gauss_decimal_precision")
                {
                    if (field.getType() != matlab::data::ArrayType::INT8 || field.getNumberOfElements() != 1)
                    {
                        throw std::invalid_argument("Invalid gauss_decimal_precision");
                    }
                    settings.gauss_decimal_precision = field[0];
                }
                else if (fieldName == "max_iterations_bfs")
                {
                    if (field.getType() != matlab::data::ArrayType::DOUBLE || field.getNumberOfElements() != 1)
                    {
                        throw std::invalid_argument("Invalid max_iterations_bfs");
                    }
                    settings.max_iterations_bfs = field[0];
                }
                else if (fieldName == "bfs_decimal_precision")
                {
                    if (field.getType() != matlab::data::ArrayType::INT8 || field.getNumberOfElements() != 1)
                    {
                        throw std::invalid_argument("Invalid bfs_decimal_precision");
                    }
                    settings.bfs_decimal_precision = field[0];
                }
                else
                {
                    throw std::invalid_argument("Invalid option " + fieldName + " in setting struct");
                }
            }
        }

        NetworkLoader loader(file);
        std::shared_ptr<Network> net = loader.loadNetwork();
        SolverSettings settings{};
        solvers.insert({ handleCounter, std::make_unique<PowerFlowSolver>(net, settings, &logger) });

        std::uint64_t handle = handleCounter;
        ++handleCounter;
        matlab::data::ArrayFactory factory;
        outputs[0] = factory.createScalar<std::uint64_t>(handle);
    }

    void solve(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs)
    {
        if (inputs.size() < 3 || inputs[2].getType() != matlab::data::ArrayType::COMPLEX_DOUBLE)
        {
            throw std::invalid_argument("Missing or invalid S vector");
        }
        if (inputs.size() < 4 || inputs[3].getType() != matlab::data::ArrayType::COMPLEX_DOUBLE && !inputs[3].isEmpty())
        {
            throw std::invalid_argument("Missing or invalid V vector");
        }

        std::unique_ptr<PowerFlowSolver>& solver = solvers.at(getSolverHandle(inputs));
        matlab::data::ArrayFactory factory;
        matlab::data::TypedArray<complex_t> matlabS = inputs[2];
        matlab::data::TypedArray<complex_t> matlabV = inputs[3];

        std::vector<complex_t> S(matlabS.begin(), matlabS.end());
        std::vector<complex_t> V(matlabV.begin(), matlabV.end());

        solver->solve(S, V);
    }

    void getLoadVoltages(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs)
    {
        std::unique_ptr<PowerFlowSolver>& solver = solvers.at(getSolverHandle(inputs));
        std::vector<complex_t> V = solver->getLoadVoltages();
        matlab::data::ArrayFactory factory;
        outputs[0] = factory.createArray({ 1, V.size() }, V.begin(), V.end());
    }

    void getAllVoltages(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs)
    {
        std::unique_ptr<PowerFlowSolver>& solver = solvers.at(getSolverHandle(inputs));
        std::vector<complex_t> V = solver->getAllVoltages();
        matlab::data::ArrayFactory factory;
        outputs[0] = factory.createArray({ 1, V.size() }, V.begin(), V.end());
    }

    void getCurrents(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs)
    {
        std::unique_ptr<PowerFlowSolver>& solver = solvers.at(getSolverHandle(inputs));
        std::vector<complex_t> I = solver->getCurrents();
        matlab::data::ArrayFactory factory;
        outputs[0] = factory.createArray({ 1, I.size() }, I.begin(), I.end());
    }

    void getSlackPowers(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs)
    {
        std::unique_ptr<PowerFlowSolver>& solver = solvers.at(getSolverHandle(inputs));
        std::vector<complex_t> S = solver->getSlackPowers();
        matlab::data::ArrayFactory factory;
        outputs[0] = factory.createArray({ 1, S.size() }, S.begin(), S.end());
    }

    void unloadNetwork(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs)
    {
        std::uint64_t handle = getSolverHandle(inputs);

        if (solvers.count(handle) == 0)
        {
            throw std::runtime_error("Network not loaded");
        }
        else
        {
            solvers.erase(handle);
        }
    }

    void printToMatlab(const std::ostringstream& message)
    {
        matlab::data::ArrayFactory factory;
        matlabPtr->feval(u"fprintf", 0, std::vector<matlab::data::Array>({ factory.createScalar(message.str()) }));
    }
};
