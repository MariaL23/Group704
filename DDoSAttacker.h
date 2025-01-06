#pragma once
#include "veins_inet/veins_inet.h"
#include "veins_inet/VeinsInetApplicationBase.h"
#include "inet/networklayer/common/L3Address.h"

namespace veins {
class DDoSAttacker : public VeinsInetApplicationBase {
protected:
    // Attack configuration
    int floodRate;
    double burstProbability;
    int burstSize;
    int packetSize;
    std::string victimId;
    double compromiseInterval;

    // Attack state
    bool attackActive;
    int attackPacketsSent;
    int compromisePacketsSent;
    inet::L3Address ownAddress;
    inet::L3Address victimAddress;  // Added to store victim's address

    // Timers
    cMessage* floodTimer;
    cMessage* compromiseTimer;
    cMessage* beaconTimer;

    // Beacon parameters
    double beaconInterval;
    int beaconPacketSize;

    // Statistics
    simsignal_t attackTrafficSignal;
    simsignal_t burstCountSignal;
    simsignal_t compromiseAttemptsSignal;
    simsignal_t beaconsSentSignal;

protected:
    virtual bool startApplication() override;
    virtual bool stopApplication() override;
    virtual void processPacket(std::shared_ptr<inet::Packet> pk) override;
    virtual void handleSelfMessage(cMessage* msg);
    virtual void sendDDoSPacket(bool isCompromise);
    virtual void sendCompromisePacket();
    virtual void sendFloodPackets(int numPackets);
    virtual void handleMessageWhenUp(cMessage* msg) override;
    virtual void sendBeacon();
    virtual void resolveVictimAddress();

public:
    DDoSAttacker();
    virtual ~DDoSAttacker();
};
} // namespace veins
