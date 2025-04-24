import python_wrappers
from scipy.optimize import minimize
#
file = "../examples/example_network_single_grid.txt"

# Skapa en PowerFlowSolver-instans baserat på nätverksfilen
solver = python_wrappers.PowerFlowWrapper(file)

def obective(x): #Funktionen att minimera 
    P = [
        complex(x[0], x[1]),
        complex(x[2], x[3]),
        complex(x[4], x[5])
    ]

    U = solver.solve(P)
    avg_mag = sum(abs(u) for u in U) / len(U) # Minimera negativt värde = maximera spänning
    return -avg_mag

x0 = [0.01, 0.01, 0.01, 0.01, 0.01, 0.01] #Gissning 

bounds = [(0, 0.2)] * 6 #Vi vill inte ha negativa eller för höga spänningsskillnader

def total_real_power(x):
    return sum(x[0::2]) - 0.15  # extraherar P1, P2, P3

constraints = [
    {'type': 'ineq', 'fun': total_real_power} # Vi tillåter inte en summa av lastvärden under 0.15
]

result = minimize(obective, x0, bounds=bounds, constraints=constraints)

P_optimized = []

# Visa de optimerade spänninsvärderna
for i in range(3): 
    real = result.x[2*i]
    imag = result.x[2*i+1]
    p_complex = complex(real, imag)
    P_optimized.append(p_complex)

    print(f"  Node {i}: P = {real:.4f}, Q = {imag:.4f}  →  {p_complex}")

# Kör solve med de optimerade lastvärderna
U_optimized = solver.solve(P_optimized)

# Visa spänningsresultatet
print("\n Spänningen med :")
for i, u in enumerate(U_optimized):
    print(f"  Node {i}: U = {u}")
