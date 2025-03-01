#Disclaimer: Detta är bara ett experiment för att se ett exempel på hur optimering av elnät kan användas i python
#och är därmed Frankensteinkod, till stor del utvecklad mha gpt. Inget som ska användas i projektet

import pandapower as pp
import numpy as np
from scipy.optimize import minimize

# Funktion för att skapa ett trädstrukturerat elnät
def create_network(tap_position, households):
    net = pp.create_empty_network()

    # Skapa huvudbuss (slack, matande nät)
    bus_slack = pp.create_bus(net, vn_kv=20, name="Slack")

    # Skapa huvudtransformator som reglerar spänning
    bus_main = pp.create_bus(net, vn_kv=0.4, name="Main Bus")
    pp.create_ext_grid(net, bus=bus_slack, vm_pu=1.02)  # Matning med 1.02 per/unit alltså 1.02*20kV, detta är referens 
    pp.create_transformer(net, hv_bus=bus_slack, lv_bus=bus_main,
                           std_type="0.63 MVA 20/0.4 kV", tap_pos=tap_position)

    # Skapa trädstruktur med hushåll som löv
    buses = [bus_main]  # Start med huvudbussen
    for i, (p_mw, q_mvar) in enumerate(households):
        bus = pp.create_bus(net, vn_kv=0.4, name=f"Household_{i}")
        pp.create_line(net, from_bus=buses[-1], to_bus=bus, length_km=0.1, std_type="NAYY 4x150 SE")
        buses.append(bus)

        # Lägg till hushållets konsumtion (positiv P och Q = last)
        if p_mw > 0:
            pp.create_load(net, bus=bus, p_mw=p_mw, q_mvar=q_mvar)

        # Lägg till hushållets produktion (negativ P = solceller)
        if p_mw < 0:
            pp.create_sgen(net, bus=bus, p_mw=-p_mw, q_mvar=-q_mvar)

    return net

# Funktion för att beräkna spänningsvariation
def voltage_spread(tap_position, households):
    net = create_network(tap_position[0], households)
    try:
        pp.runpp(net, init='dc', max_iteration=50, tolerance_mva=1e-3, enforce_q_lims=True)
        voltages = net.res_bus.vm_pu
        return np.max(voltages) - np.min(voltages)  # Minimera Vmax - Vmin
    except pp.powerflow.LoadflowNotConverged:
        return 1e6  # Ge högt straff om power flow inte konvergerar

# Exempel: Lista med hushåll där (P, Q) är effektförbrukning (positiv) eller produktion (negativ)
households = [
    (0.01, 0.002),  # Hushåll 1: Konsumerar 10 kW, 2 kvar
    (0.015, 0.003),  # Hushåll 2: Konsumerar 15 kW, 3 kvar
    (-0.005, -0.001),  # Hushåll 3: Producerar 5 kW solkraft
    (0.02, 0.004),  # Hushåll 4: Konsumerar 20 kW
    (-0.01, -0.002)  # Hushåll 5: Producerar 10 kW solkraft
]

# Startvärde och begränsningar för transformatorns tap-läge
x0 = np.array([0])
bounds = [(-2, 2)]  # Tap-position mellan -2 och 2 steg

# Kör optimering för att minimera spänningsspridningen
result = minimize(voltage_spread, x0, args=(households,), method='SLSQP', bounds=bounds)

# Visa resultat
optimal_tap = result.x[0]
final_spread = voltage_spread([optimal_tap], households)
print(f"Optimal tap-position: {optimal_tap:.2f}")
print(f"Minimal spänningsskillnad |Vmax - Vmin|: {final_spread:.4f} p.u.")
