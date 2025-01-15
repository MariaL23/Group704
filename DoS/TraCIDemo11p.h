#pragma once

#include "veins/modules/application/ieee80211p/DemoBaseApplLayer.h"
#include "veins/modules/application/traci/TraCIDemo11pMessage_m.h"

namespace veins {

/**
 * @brief
 * A tutorial demo for TraCI. When the car is stopped for longer than 10 seconds
 * it will send a message out to other cars containing the blocked road id.
 * Receiving cars will then trigger a reroute via TraCI.
 * When channel switching between SCH and CCH is enabled on the MAC, the message is
 * instead send out on a service channel following a Service Advertisement
 * on the CCH.
 *
 * @author Christoph Sommer : initial DemoApp
 * @author David Eckhoff : rewriting, moving functionality to DemoBaseApplLayer, adding WSA
 *
 */

class VEINS_API TraCIDemo11p : public DemoBaseApplLayer {
public:
    void initialize(int stage) override;
    void finish() override;

protected:
    // Member variables
    simtime_t lastDroveAt;
    simtime_t bandwidthStartTime;
    simtime_t lastEmergencyMessageReceived;

    bool sentMessage;
    bool yieldingForEmergency = false; // Flag to track yielding state

    int currentSubscribedServiceId;
    int wsmSentCount = 0; // Tracks the number of messages sent by this node
    int latencyCount = 0;
    int floodMessage;
    int normalMessage;

    long totalBytesSent = 0;
    long totalPacketsSent = 0;
    long totalPacketsReceived = 0;

    double totalLatency = 0.0; // Sum of all latencies
    double maxLatency = 0.0;   // Maximum observed latency
    const double yieldingTime = 100.0; // Second interval

    // Signals
    simsignal_t latencySignal;    // Signal to track latency
    simsignal_t throughputSignal; // Signal to track throughput
    simsignal_t trafficSignal;    // Signal to track traffic
    simsignal_t generatedMessageSignal; // Signal for message generation

    // Member functions
    void onWSM(BaseFrame1609_4* wsm) override;
    void onWSA(DemoServiceAdvertisment* wsa) override;
    void handleSelfMsg(cMessage* msg) override;
    void handlePositionUpdate(cObject* obj) override;
    void handleNode1Flooding();
    void handleDNSFlooding();

    // Utility functions
    bool validateResponse(TraCIDemo11pMessage* wsm);
};

} // namespace veins
