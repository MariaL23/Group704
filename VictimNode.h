#pragma once
#include "veins_inet/veins_inet.h"
#include "veins_inet/VeinsInetApplicationBase.h"
#include "inet/networklayer/common/L3Address.h"
#include <queue>
#include <map>

namespace veins {
class VictimNode : public veins::VeinsInetApplicationBase {
protected:
    // Configuration
    int packetSize;
    int attackPacketsSent;
    inet::L3Address ownAddress;

    // Statistics signals
    simsignal_t packetRateSignal;
    simsignal_t queueLengthSignal;
    simsignal_t processingDelaySignal;
    simsignal_t bytesReceivedSignal;
    simsignal_t beaconsSentSignal;

    // Timers
    cMessage* statsTimer;
    cMessage* beaconTimer;

    // Tracking variables
    std::map<simtime_t, int> packetCounter;
    std::queue<std::shared_ptr<inet::Packet>> processingQueue;
    simtime_t lastProcessingTime;

    // Beacon parameters
    double beaconInterval;
    int beaconPacketSize;

protected:
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage* msg) override;
    virtual void handleSelfMessage(omnetpp::cMessage* msg);
    virtual bool startApplication() override;
    virtual bool stopApplication() override;
    virtual void processPacket(std::shared_ptr<inet::Packet> pk) override;
    virtual void updateStatistics();
    virtual void sendBeacon();

public:
    VictimNode();
    virtual ~VictimNode();
};
}
