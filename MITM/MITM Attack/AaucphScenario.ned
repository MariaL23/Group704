//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

import org.car2x.veins.nodes.RSU;
import org.car2x.veins.nodes.Scenario;
import org.car2x.veins.visualizer.roads.RoadsOsgVisualizer;
import org.car2x.veins.modules.application.traci.MITMAttackNode;

network AaucphScenario extends Scenario
{
    parameters:
        bool useOsg = true;
        @display("bgb=3000.2542,3000.644");
    submodules:
        rsu[2]: RSU {
            @display("p=566,32;i=veins/sign/yellowdiamond;is=vs");
        }
        roadsOsgVisualizer: RoadsOsgVisualizer if useOsg {
            @display("p=287,32;i=veins/node/car;is=vs");
        }
}