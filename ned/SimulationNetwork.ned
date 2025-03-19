package nr.v2x;

// Import required packages
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.visualizer.integrated.IntegratedVisualizer;
import simu5g.nodes.NR.NRUe;
import simu5g.nodes.NR.gNodeB;
import simu5g.world.radio.LteChannelControl;
import veins.base.modules.BaseWorldUtility;

network SimulationNetwork
{
    parameters:
        @display("bgb=1000,1000;bgi=background/urban");
        
        // Playground dimensions
        double playgroundSizeX @unit(m) = default(1000m);
        double playgroundSizeY @unit(m) = default(1000m);
        
        // Number of vehicles/UEs
        int numVehicles = default(10);
        
    submodules:
        // World utility (from Veins) for coordination
        world: BaseWorldUtility {
            parameters:
                @display("p=50,50");
        }
        
        // Channel controller for NR
        channelControl: LteChannelControl {
            parameters:
                @display("p=50,150");
        }
        
        // Network configurator for IP addressing
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=50,250");
        }
        
        // Integrated visualizer for network visualization
        visualizer: IntegratedVisualizer {
            parameters:
                @display("p=50,350");
        }
        
        // gNodeB (5G base station)
        gNodeB: gNodeB {
            parameters:
                @display("p=300,200;is=vl");
        }
        
        // Vehicle UEs (User Equipment)
        vehicle[numVehicles]: NRUe {
            parameters:
                @display("p=200,300,row,100;i=veins/node/car");
                mobility.typename = "VeinsInetMobility";  // Use Veins mobility model
        }
        
    connections allowunconnected:
        // Connections will be handled dynamically by the channel controller
}