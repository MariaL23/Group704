#include "veins/modules/application/traci/TraCIDemo11p.h"
#include "veins/modules/application/traci/TraCIDemo11pMessage_m.h"

using namespace veins;

Define_Module(veins::TraCIDemo11p);

void TraCIDemo11p::initialize(int stage) {
    DemoBaseApplLayer::initialize(stage);
    if (stage == 0) {
        sentMessage = false;
        lastDroveAt = simTime();
        currentSubscribedServiceId = -1;

        mobility = TraCIMobilityAccess().get(getParentModule());
        traci = mobility->getCommandInterface();
    }
}

void TraCIDemo11p::onWSM(BaseFrame1609_4* frame) {
    TraCIDemo11pMessage* wsm = dynamic_cast<TraCIDemo11pMessage*>(frame);
    if (!wsm) return;

    bool isTampered = wsm->hasPar("isTampered") && wsm->par("isTampered").boolValue();

    if (isTampered) {
        EV << "Vehicle received tampered message at " << simTime() << endl;

        if (mobility && traci) {
            auto vehicle = traci->vehicle(mobility->getExternalId());

            // Force stop
            vehicle.setSpeed(0.0);
            vehicle.setSpeedMode(0x00);  // Disable all speed checks

            // Visual indication
            findHost()->getDisplayString().setTagArg("i", 1, "red");
            findHost()->getDisplayString().setTagArg("t", 0, "STOPPED by Attack!");

            // Create new periodic stop event if not exists
            if (!stopMessageEvent) {
                stopMessageEvent = new cMessage("maintainStop");
                scheduleAt(simTime() + 0.1, stopMessageEvent);  // More frequent updates
            }

            EV << "Vehicle " << mobility->getExternalId() << " stopped by attack" << endl;
        }
    }
}

void TraCIDemo11p::onWSA(DemoServiceAdvertisment* wsa) {
    // Check if we need to stop current service before starting new one
    if (currentSubscribedServiceId != -1 && currentSubscribedServiceId != wsa->getPsid()) {
        stopService();  // Stop existing service if it's different
    }

    // Only start a new service if none is running
    if (currentSubscribedServiceId == -1) {
        mac->changeServiceChannel(static_cast<Channel>(wsa->getTargetChannel()));
        startService(static_cast<Channel>(wsa->getTargetChannel()), wsa->getPsid(), "Mirrored Traffic Service");
        currentSubscribedServiceId = wsa->getPsid();
    }
}

void TraCIDemo11p::handlePositionUpdate(cObject* obj) {
    DemoBaseApplLayer::handlePositionUpdate(obj);

    if (mobility->getSpeed() < 1) {
        if (simTime() - lastDroveAt >= 10 && !sentMessage) {
            findHost()->getDisplayString().setTagArg("i", 1, "red");
            sentMessage = true;

            TraCIDemo11pMessage* wsm = new TraCIDemo11pMessage();
            populateWSM(wsm);
            wsm->setDemoData(mobility->getRoadId().c_str());

            if (dataOnSch) {
                if (currentSubscribedServiceId != -1) {
                    stopService();  // Stop existing service before starting new one
                }
                startService(Channel::sch2, 42, "Traffic Info");
                currentSubscribedServiceId = 42;  // Track the service ID
                scheduleAt(computeAsynchronousSendingTime(1, ChannelType::service), wsm);
            } else {
                sendDown(wsm);
            }
        }
    } else {
        lastDroveAt = simTime();
    }
}

void TraCIDemo11p::handleSelfMsg(cMessage* msg) {
    if (msg == stopMessageEvent) {
        if (mobility && traci) {
            // Keep reinforcing the stop
            auto vehicle = traci->vehicle(mobility->getExternalId());
            vehicle.setSpeed(0.0);
            vehicle.setSpeedMode(0x00);

            // Schedule next stop reinforcement
            scheduleAt(simTime() + 0.1, msg);  // More frequent updates
        }
    } else {
        DemoBaseApplLayer::handleSelfMsg(msg);
    }
}

void TraCIDemo11p::handleLowerMsg(cMessage* msg) {
    BaseFrame1609_4* wsm = dynamic_cast<BaseFrame1609_4*>(msg);
    if (wsm) {
        onWSM(wsm);
    }
    delete msg;
}
void TraCIDemo11p::finish() {
    // Clean up stopMessageEvent if exists
    if (stopMessageEvent) {
        if (stopMessageEvent->isScheduled()) {
            cancelEvent(stopMessageEvent);
        }
        delete stopMessageEvent;
        stopMessageEvent = nullptr;
    }
    DemoBaseApplLayer::finish();
}

