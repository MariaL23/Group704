package org.car2x.veins.subprojects.veins_inet;

simple DDoSAttacker extends VeinsInetApplicationBase
{
    parameters:
        @class(veins::DDoSAttacker);
        
        // Attack configuration
        int floodRate = default(10);          
        double burstProbability = default(0.3); 
        int burstSize = default(50);           
        int packetSize @unit(B) = default(1000B);
        string victimId = default("13");        
        double compromiseInterval @unit(s) = default(0.5s);
        
        // Beacon parameters
        double beaconInterval @unit(s) = default(1s);
        int beaconPacketSize @unit(B) = default(500B);
        
        // Statistics signals
        @signal[attackTraffic](type=long);
        @signal[burstCount](type=long);
        @signal[compromiseAttempts](type=long);
        @signal[beaconsSent](type=long);
        
        // Statistics recording
        @statistic[attackPackets](title="attack packets sent"; source=attackTraffic; record=count,sum,vector);
        @statistic[bursts](title="burst events"; source=burstCount; record=count,vector);
        @statistic[compromiseAttempts](title="compromise attempts"; source=compromiseAttempts; record=count,vector);
        @statistic[beaconsSent](title="Beacons sent"; source=beaconsSent; record=count,vector);
}