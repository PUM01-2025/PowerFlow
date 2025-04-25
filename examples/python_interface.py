import python_wrappers
from scipy.optimize import minimize

file = "../examples/example_network_2.txt"
solver = python_wrappers.PowerFlow(file)

def objective(x):
    P = [complex(x[2*i], x[2*i+1]) for i in range(3)]
    U = solver.solve(P)
    avg_mag = sum(abs(u) for u in U) / len(U)
    return -avg_mag

def total_real_power(x):
    return sum(x[0::2]) - 0.15

if __name__ == "__main__":
    x0 = [0.01, 0.01, 0.01, 0.01, 0.01, 0.01]
    bounds = [(0, 0.2)] * 6
    constraints = [{'type': 'ineq', 'fun': total_real_power}]

    result = minimize(objective, x0, bounds=bounds, constraints=constraints)

    print("\nOptimerade lastvärden:")
    P_optimized = []
    for i in range(3):
        real = result.x[2*i]
        imag = result.x[2*i+1]
        p_complex = complex(real, imag)
        P_optimized.append(p_complex)
        print(f"  Node {i}: P = {real:.4f}, Q = {imag:.4f}  PQ =  {p_complex}")

    U_optimized = solver.solve(P_optimized)
    print("\nSpänningen med optimerade lastvärden:")
    for i, u in enumerate(U_optimized):
        print(f"  Node {i}: U = {u}")
