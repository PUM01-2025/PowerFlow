import PowerFlowPython

settings = PowerFlowPython.SolverSettings()
settings.max_iterations_total = 1000
net = PowerFlowPython.PowerFlow("example_network.txt", settings)

S = [0.002 + 0.001j, 0.005 + 0.004j, 0.004 + 0.002j]
V_ext = [1]

net.solve(S, V_ext)

V = net.getLoadVoltages()
print(V)

V_all = net.getAllVoltages()
print(V_all)

I = net.getCurrents()
print(I)

S_slack = net.getSlackPowers()
print(S_slack)

net.reset()
