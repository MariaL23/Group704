#pragma once

#include "veins/modules/application/ieee80211p/DemoBaseApplLayer.h"
#include "veins/modules/application/traci/TraCIDemo11pMessage_m.h"
#include "veins/modules/mobility/traci/TraCIMobility.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"

namespace veins {

class VEINS_API MITMAttackNode : public DemoBaseApplLayer {
protected:
    // Add these declarations
    simtime_t lastDroveAt;  // Time when the node last drove
    bool sentMessage;       // Flag to track if message was sent

    TraCIMobility* mobility;
    TraCICommandInterface* traci;
    cMessage* attackEvent;
    double attackRadius;

    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual void handleSelfMsg(cMessage* msg) override;
    virtual void onWSM(BaseFrame1609_4* wsm) override;
    virtual void handlePositionUpdate(cObject* obj) override;
    void launchAttack();
    virtual void handleLowerMsg(cMessage* msg) override;

public:
    MITMAttackNode() : lastDroveAt(0), sentMessage(false),
                       mobility(nullptr), traci(nullptr),
                       attackEvent(nullptr), attackRadius(0) {}
    virtual ~MITMAttackNode();
};

} // namespace veins
