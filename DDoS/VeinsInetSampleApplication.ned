package org.car2x.veins.subprojects.veins_inet;

simple VeinsInetSampleApplication extends VeinsInetApplicationBase
{
    parameters:
        @class(veins::VeinsInetSampleApplication);
        
        // Attack parameters
        int floodRate = default(100);          // Packets per second in normal mode
        double burstProbability = default(0.3); // Probability of sending a burst
        int burstSize = default(50);           // Number of packets in a burst
        int packetSize @unit(B) = default(1000B); // Size of each packet
        
        // Beacon parameters
        double beaconInterval @unit(s) = default(1s);
        int beaconPacketSize @unit(B) = default(500B);
        
        // Statistics signals
        @signal[packetRate](type=double);
        @signal[queueLength](type=long);
        @signal[processingDelay](type=simtime_t);
        @signal[bytesReceived](type=long);
        @signal[beaconsSent](type=long);
        @signal[attackTraffic](type=long);
        @signal[burstCount](type=long);
        
        // Statistics recording
        @statistic[packetRate](title="Packets per second"; source=packetRate; record=vector,timeavg,max,count);
        @statistic[queueLength](title="Queue Length"; source=queueLength; record=vector,timeavg,max);
        @statistic[processingDelay](title="Processing Delay"; source=processingDelay; record=vector,timeavg,max);
        @statistic[bytesReceived](title="Bytes Received"; source=bytesReceived; record=vector,sum,timeavg);
        @statistic[beaconsSent](title="Beacons sent"; source=beaconsSent; record=count,vector);
        @statistic[attackPackets](title="Attack packets sent"; source=attackTraffic; record=count,vector);
        @statistic[bursts](title="Burst events"; source=burstCount; record=count,vector);
}