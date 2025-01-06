package org.car2x.veins.subprojects.veins_inet;

simple VictimNode extends VeinsInetApplicationBase
{
    parameters:
        @class(veins::VictimNode);
        
        // Configuration
        int packetSize @unit(B) = default(1000B);
        
        // Beacon parameters
        double beaconInterval @unit(s) = default(1s);
        int beaconPacketSize @unit(B) = default(500B);
        
        // Signal declarations
        @signal[packetRate](type=double);
        @signal[queueLength](type=long);
        @signal[processingDelay](type=simtime_t);
        @signal[bytesReceived](type=long);
        @signal[beaconsSent](type=long);
        
        // Statistics recording
        @statistic[packetRate](title="Packets per second"; source=packetRate; record=vector,timeavg,max,count);
        @statistic[queueLength](title="Queue Length"; source=queueLength; record=vector,timeavg,max);
        @statistic[processingDelay](title="Processing Delay"; source=processingDelay; record=vector,timeavg,max);
        @statistic[bytesReceived](title="Bytes Received"; source=bytesReceived; record=vector,sum,timeavg);
        @statistic[beaconsSent](title="Beacons sent"; source=beaconsSent; record=count,vector);
}