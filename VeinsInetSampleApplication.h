#pragma once
#include "veins_inet/veins_inet.h"
#include "veins_inet/VeinsInetApplicationBase.h"
#include "inet/common/TagBase_m.h"
#include "inet/common/Protocol.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include <cmath>
#include <queue>
#include <map>
#include "inet/common/InitStages.h"


namespace veins {
using namespace inet;

class VeinsInetSampleApplication : public VeinsInetApplicationBase {
protected:
    // Constants
    static const int MAX_INIT_RETRIES = 10;

    // Attack configuration
    int floodRate;
    double burstProbability;
    int burstSize;
    int packetSize;
    bool compromised;
    int attackPacketsSent;
    std::string targetVictimId;
    std::string myId;

    // Network state
    inet::L3Address ownAddress;
    inet::L3Address victimAddress;
    int initRetryCount;

    // Timers
    cMessage* floodTimer;
    cMessage* statsTimer;
    cMessage* beaconTimer;

    // Beacon parameters
    double beaconInterval;
    int beaconPacketSize;

    // Statistics signals
    simsignal_t attackTrafficSignal;
    simsignal_t burstCountSignal;
    simsignal_t packetRateSignal;
    simsignal_t queueLengthSignal;
    simsignal_t processingDelaySignal;
    simsignal_t bytesReceivedSignal;
    simsignal_t beaconsSentSignal;

    // Tracking variables
    std::map<simtime_t, int> packetCounter;
    std::queue<std::shared_ptr<inet::Packet>> processingQueue;
    simtime_t lastProcessingTime;

protected:
    virtual void initializeAddress();
    virtual void scheduleInitRetry();
    virtual void sendFloodPackets(int numPackets);
    virtual void sendDDoSPacket();
    virtual void sendBeacon();
    virtual int numInitStages() const override;
    virtual void updateStatistics();

public:
    VeinsInetSampleApplication();
    virtual ~VeinsInetSampleApplication();

protected:
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage* msg) override;
    virtual void processPacket(std::shared_ptr<inet::Packet> pk) override;
    virtual bool startApplication() override;
    virtual bool stopApplication() override;
};
}
