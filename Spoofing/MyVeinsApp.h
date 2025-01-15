#pragma once

#include "veins/veins.h"

#include "veins/modules/application/traci/TraCIDemo11p.h"

using namespace omnetpp;

namespace veins {

class VEINS_API MyVeinsApp : public TraCIDemo11p {
public:
    void initialize(int stage) override;
    void finish() override;

protected:

//    Coord AttackerPosition;
    simtime_t broadcastInterval = 20; // second interval
    cMessage* broadcastMessageEvent; // Timer event for message broadcasting


    void onBSM(DemoSafetyMessage* bsm) override;
    void onWSM(BaseFrame1609_4* wsm) override;
    void onWSA(DemoServiceAdvertisment* wsa) override;

    void handleSelfMsg(cMessage* msg) override;
    void handlePositionUpdate(cObject* obj) override;
    virtual void sendEmergencyMessage();
};

} // namespace veins
