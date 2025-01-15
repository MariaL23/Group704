package grp704_project.simulations.veins;
import org.car2x.veins.nodes.Scenario;
import inet.visualizer*.integrated.IntegratedVisualizer;
import inet.environment.common.PhysicalEnvironment;
import org.car2x.veins.visualizer.roads.RoadsOsgVisualizer;
import org.car2x.veins.modules.application.ieee80211p.DemoBaseApplLayer;
//import veins.base.modules.BaseModule;
network AaucphScenario extends Scenario
{
    parameters:
        bool useOsg = true;  
            submodules:
        visualizer: IntegratedVisualizer {
            @display("p=293,359");
        }
        physicalEnvironment: PhysicalEnvironment {
            @display("p=29,359");
        }
        roadsOsgVisualizer: RoadsOsgVisualizer if useOsg {
            @display("p=192,416");
        }
}

