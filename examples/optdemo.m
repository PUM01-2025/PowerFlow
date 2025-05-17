%% Optimeringsexempel 1

clear all;
close all;

% Baser.
S_base = 1000000;
V_base = 400;
% Z_base = V_base^2/S_base

% Beräknar spänningen i varje tidpunkt m.h.a. C++-lösaren.
function [V] = solveSeries(net, S)
    V = zeros(1, length(S));
    for i=1:length(S)
        V_ext = sin(((2*pi)/24)*i)/50 + 1; % Spänning varierar mellan 408 och 392 V under 24h
        net.solve(complex(S(i)), complex(V_ext));
        V(i) = net.getLoadVoltages();
    end
end

% Målfunktionen som ska minimeras: |Vmax - Vmin|.
function [Vdiff] = voltageDiff(net, S, S_known, V_base)
    V = abs(solveSeries(net, S + S_known)*V_base);
    Vdiff = range(V);
end

% Läs in exempelnätverket från fil.
net = PowerFlow("grid_basic.txt", []);

% Skapa målfunktionen med påhittad känd effektförbrukning S_known.
S_known = [400, 500, 70, 100, 200, 300, 1000, 2000, 4000, 2000, 3000, 1000]/S_base;
N = length(S_known);
f = @(S)voltageDiff(net, S, S_known, V_base); % Målfunktionen f.

% Aeq och beq skickas till fmincon och definierar villkoret Aeq*x=beq.
Aeq = ones(1, N); % = [1 1 1 1 ...]
beq = 70000/S_base; % sum(P) = 70 kWh
lb = 0/S_base*ones(1, N); % Undre gräns 0 för effekterna
S0 = 40000/N/S_base * ones(1, N); % Initial gissning S0: Jämnt fördelad laddning
options = optimoptions('fmincon', ...
    'Algorithm','interior-point', ...
                       'Display', 'iter', ...
                       'MaxFunctionEvaluations',1e5, ...
                       'ConstraintTolerance',1e-15, ...
                       'StepTolerance',1e-32, ...
                       'FunctionTolerance',1e-15, ...
                       'OptimalityTolerance',1e-15, ...
                       'FiniteDifferenceType','central');

S = fmincon(f, S0, [], [], Aeq, beq, lb, [], [], options)

V = abs(solveSeries(net, (S + S_known))*V_base)
S_bad = 70000/N/S_base * ones(1, N);
V_bad = abs(solveSeries(net, (S_bad + S_known))*V_base)
V_diff = abs(max(V) - min(V))
V_bad_diff = abs(max(V_bad) - min(V_bad))
