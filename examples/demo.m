settings.max_iterations_total = 1000;
net = PowerFlow("example_network.txt", settings);

S = complex([0.002 + 0.001i, 0.005 + 0.004i, 0.004 + 0.002i]);
V_ext = complex(1);

net.solve(S, V_ext);

V = net.getLoadVoltages()

V_all = net.getAllVoltages()

I = net.getCurrents()

S_slack = net.getSlackPowers()

net.reset()
