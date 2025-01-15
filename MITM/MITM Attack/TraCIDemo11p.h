#pragma once

#include "veins/modules/application/ieee80211p/DemoBaseApplLayer.h"
#include "veins/modules/application/traci/TraCIDemo11pMessage_m.h"
#include "veins/modules/mobility/traci/TraCIMobility.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"

namespace veins {

class VEINS_API TraCIDemo11p : public DemoBaseApplLayer {
protected:
    simtime_t lastDroveAt;
    bool sentMessage;
    int currentSubscribedServiceId;
    TraCIMobility* mobility;
    TraCICommandInterface* traci;
    cMessage* stopMessageEvent = nullptr;

    virtual void initialize(int stage) override;
    virtual void handleSelfMsg(cMessage* msg) override;
    virtual void handlePositionUpdate(cObject* obj) override;
    virtual void handleLowerMsg(cMessage* msg) override;  // Added this
    virtual void onWSM(BaseFrame1609_4* wsm) override;
    virtual void onWSA(DemoServiceAdvertisment* wsa) override;
    virtual void finish()override ;

public:
    TraCIDemo11p() {
        sentMessage = false;
        lastDroveAt = simTime();
        currentSubscribedServiceId = -1;
    }

    virtual ~TraCIDemo11p() {
        if (stopMessageEvent) {
            if (stopMessageEvent->isScheduled()) {
                cancelEvent(stopMessageEvent);
            }
            delete stopMessageEvent;
        }
    }
};

} // namespace veins
