// AaucphScenario.ned
package org.car2x.veins.subprojects.veins_inet;

import inet.physicallayer.ieee80211.packetlevel.Ieee80211DimensionalRadioMedium;
import org.car2x.veins.subprojects.veins_inet.VeinsInetCar;
import org.car2x.veins.subprojects.veins_inet.VeinsInetManager;
import inet.visualizer*.integrated.IntegratedVisualizer;
import inet.environment.common.PhysicalEnvironment;
import org.car2x.veins.visualizer.roads.RoadsCanvasVisualizer;
import org.car2x.veins.visualizer.roads.RoadsOsgVisualizer;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;

network AaucphScenario
{
    parameters:
        bool useOsg = true;
        @display("bgb=4000,4000");
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=192,128");
            assignAddresses = true;
            addDefaultRoutes = true;
            optimizeRoutes = true;
            assignDisjunctSubnetAddresses = false;
            addSubnetRoutes = true;
            dumpConfig = "true";  // This will help us debug
            dumpAddresses = true;  // This will help us debug
        }
        radioMedium: Ieee80211DimensionalRadioMedium {
            @display("p=64,224");
        }
        manager: VeinsInetManager {
            @display("p=192,320");
        }
        visualizer: IntegratedVisualizer {
            @display("p=64,320");
        }
        physicalEnvironment: PhysicalEnvironment {
            @display("p=192,224");
        }
        roadsCanvasVisualizer: RoadsCanvasVisualizer {
            @display("p=64,416");
        }
        roadsOsgVisualizer: RoadsOsgVisualizer if useOsg {
            @display("p=192,416");
        }
        node[0]: VeinsInetCar {
            parameters:
                @display("p=50,50");
        }
    connections allowunconnected:
}