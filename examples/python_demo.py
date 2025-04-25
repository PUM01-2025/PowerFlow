import python_wrappers
from scipy.optimize import minimize

file = "../examples/example_network_single_grid.txt"
solver = python_wrappers.PowerFlow(file)

def objective(x):
    S = [complex(x[2*i], x[2*i+1]) for i in range(3)]
    U = solver.solve(S, V_complex)
    avg_mag = sum(abs(u) for u in U) / len(U)
    return -avg_mag

def total_real_power(x):
    return sum(x[0::2]) - 0.15

if __name__ == "__main__":
    x0 = [0.01, 0.01, 0.01, 0.01, 0.01, 0.01]
    bounds = [(0, 0.2)] * 6
    constraints = [{'type': 'ineq', 'fun': total_real_power}]
    V_complex = [complex(1.0, 0.0)]
    result = minimize(objective, x0, bounds=bounds, constraints=constraints)

    print("\nOptimerade lastvärden:")
    S_optimized = []
    for i in range(3):
        real = result.x[2*i]
        imag = result.x[2*i+1]
        S_complex = complex(real, imag)
        S_optimized.append(S_complex)
        print(f"  Node {i}: P = {real:.4f}, Q = {imag:.4f}  V =  {V_complex}")

    U_optimized = solver.solve(S_optimized, V_complex)
    print("\nSpänningen med optimerade lastvärden:")
    for i, u in enumerate(U_optimized):
        print(f"  Node {i}: U = {u}")
