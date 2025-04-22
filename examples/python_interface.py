import python_wrappers

file_path = "../examples/example_network_single_grid.txt"

solver = python_wrappers.PowerFlowWrapper(file_path)

P = [1.0 + 0.5j, 0.7 + 0.2j, 0.9 + 0.4j]

U = solver.solve(P)


for i in U: 
    print(i)