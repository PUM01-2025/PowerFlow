classdef PowerFlow < handle
    properties (SetAccess = private, Hidden = true)
        networkHandle;
        loaded = false;
    end
    methods
        function this = PowerFlow(networkFilePath)
            this.networkHandle = PowerFlowMex("load", networkFilePath);
            this.loaded = true;
        end
        
        function delete(this)
            if this.loaded
                PowerFlowMex("unload", this.networkHandle);
                this.loaded = false;
            end
        end

        function [V_out] = solve(this, S, V)
            V_out = PowerFlowMex("solve", this.networkHandle, S, V);
        end
    end
end
