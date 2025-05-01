classdef PowerFlow < handle
    properties (SetAccess = private, Hidden = true)
        networkHandle;
        loaded = false;
    end
    methods
        function this = PowerFlow(networkFilePath, settings)
            if isempty(settings)
                this.networkHandle = PowerFlowMex("load", networkFilePath);
            else
                this.networkHandle = PowerFlowMex("load", networkFilePath, settings);
            end
            this.loaded = true;
        end
        
        function delete(this)
            if this.loaded
                PowerFlowMex("unload", this.networkHandle);
                this.loaded = false;
            end
        end

        function solve(this, S, V)
            PowerFlowMex("solve", this.networkHandle, S, V);
        end

        function [V] = getLoadVoltages(this)
            V = PowerFlowMex("getLoadVoltages", this.networkHandle);
        end

        function [V] = getAllVoltages(this)
            V = PowerFlowMex("getAllVoltages", this.networkHandle);
        end

        function [I] = getCurrents(this)
            I = PowerFlowMex("getCurrents", this.networkHandle);
        end

        function [S] = getSlackPowers(this)
            S = PowerFlowMex("getSlackPowers", this.networkHandle);
        end
    end
end
