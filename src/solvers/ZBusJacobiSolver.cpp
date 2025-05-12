#include "powerflow/solvers/ZBusJacobiSolver.hpp"
#include "powerflow/network.hpp"

#include <Eigen/Core>
#include <Eigen/LU>

ZBusJacobiSolver::ZBusJacobiSolver(Grid* grid, Logger* const logger, int maxIter, double precision) : GridSolver(grid, logger, maxIter, precision)
{
	// Check impedances.
	for (GridEdge& edge : grid->edges)
	{
		if (edge.z_c == 0.0)
		{
			throw std::runtime_error("Invalid 0 impedance detected in ZBusJacobiSolver");
		}
	}

	node_idx_t N = grid->nodes.size();
	Eigen::MatrixXcd ybus = Eigen::MatrixXcd::Zero(N, N);

	for (node_idx_t nodeIdx = 0; nodeIdx < grid->nodes.size(); ++nodeIdx) // "For each row in admittance matrix"
	{
		GridNode& node = grid->nodes.at(nodeIdx);
		
		if (node.type == NodeType::SLACK || node.type == NodeType::SLACK_EXTERNAL)
		{
			if (slackNodeIdx != -1)
			{
				throw std::runtime_error("Multiple slack nodes detected in grid passed to ZBusJacobiSolver");
			}
			ybus(nodeIdx, nodeIdx) = 1;
			slackNodeIdx = nodeIdx; // Store slack node index for easy access later
		}
		else 
		{
			for (edge_idx_t edgeIdx : grid->nodes.at(nodeIdx).edges) // "For each column in admittance matrix"
			{
				GridEdge& edge = grid->edges.at(edgeIdx);
				node_idx_t neighborIdx = (edge.child == nodeIdx) ? edge.parent : edge.child;

				complex_t y = (complex_t)1 / edge.z_c;
				ybus(nodeIdx, neighborIdx) = y;
				ybus(nodeIdx, nodeIdx) -= y;
			}
		}
	}

	Z = ybus.inverse();
	int zeros = 0;

	for (int row = 0; row < N; ++row)
	{
		for (int col = 0; col < N; ++col)
		{
			if (Z(row, col) == 0.0)
			{
				zeros++;
			}
		}
	}
}

int ZBusJacobiSolver::solve()
{
	node_idx_t N = grid->nodes.size();
	Eigen::VectorXcd V(N);
	Eigen::VectorXcd S(N);
	Eigen::VectorXcd I(N);

	// Transfer node voltages and powers to V and S vectors.
	for (node_idx_t nodeIdx = 0; nodeIdx < N; ++nodeIdx)
	{
		if (grid->nodes[nodeIdx].type == SLACK || grid->nodes[nodeIdx].type == SLACK_EXTERNAL)
		{
			// Slack node power is set to V*conj(V) in ZBus Jacobi.
			S(nodeIdx) = grid->nodes[nodeIdx].v * std::conj(grid->nodes[nodeIdx].v);
		}
		else
		{
			S(nodeIdx) = -grid->nodes[nodeIdx].s;
		}
		V(nodeIdx) = grid->nodes[nodeIdx].v;
	}

	int iter = 0;
	do
	{
		I = S.cwiseQuotient(V).conjugate();
		V = Z * I;
		double diff = (V.cwiseProduct(I.conjugate()) - S).cwiseAbs().maxCoeff();

		if (diff < precision)
			break;
	} while (iter++ < maxIterations); // Fel räkning av iter!! Samma i GS och BFS!!

	// Store calculated node voltages.
	for (node_idx_t nodeIdx = 0; nodeIdx < N; ++nodeIdx)
	{
		if (nodeIdx != slackNodeIdx)
		{
			grid->nodes[nodeIdx].v = V(nodeIdx);
		}
	}

	// Update the slack node power.
	updateSlackPower();
	return iter;
}

void ZBusJacobiSolver::updateSlackPower()
{
	complex_t yv = 0;
	complex_t ySum = 0;
	GridNode& slackNode = grid->nodes[slackNodeIdx];

	for (edge_idx_t edgeIdx : slackNode.edges)
	{
		GridEdge& edge = grid->edges[edgeIdx];
		int neighborIdx = edge.parent == slackNodeIdx ? edge.child : edge.parent;
		GridNode& neighbor = grid->nodes[neighborIdx];

		complex_t y = (complex_t)1 / edge.z_c;
		yv -= neighbor.v * y;
		ySum += y;
	}
	yv += slackNode.v * ySum;
	slackNode.s = slackNode.v * std::conj(yv);
}
