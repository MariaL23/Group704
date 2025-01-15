
#include "veins/modules/application/traci/TraCIDemo11p.h"

#include "veins/modules/application/traci/TraCIDemo11pMessage_m.h"

using namespace veins;

Define_Module(veins::TraCIDemo11p);

void TraCIDemo11p::initialize(int stage)
{
    DemoBaseApplLayer::initialize(stage);
    if (stage == 0) {

        cModule* appl = getParentModule()->getSubmodule("appl");
        appl->par("dataOnSch") = true;

        sentMessage = false;
        lastDroveAt = simTime();
        currentSubscribedServiceId = -1;
    }
}

void TraCIDemo11p::onWSA(DemoServiceAdvertisment* wsa)
{
    if (currentSubscribedServiceId == -1) {
        mac->changeServiceChannel(static_cast<Channel>(wsa->getTargetChannel()));
        currentSubscribedServiceId = wsa->getPsid();
        if (currentOfferedServiceId != wsa->getPsid()) {
            stopService();
            startService(static_cast<Channel>(wsa->getTargetChannel()), wsa->getPsid(), "Mirrored Traffic Service");
        }
    }
}

void TraCIDemo11p::onWSM(BaseFrame1609_4* frame)
{
    TraCIDemo11pMessage* wsm = check_and_cast<TraCIDemo11pMessage*>(frame);
    EV_INFO << "Node [" << getParentModule()->getIndex()
                    << "] received a message from Node"<< wsm->getName()  << "  \n";

    if (wsm->getKind() == 1) { // Assuming "1" is the kind for emergency messages
        EV_INFO << "Vehicle [" << getParentModule()->getIndex() << "] received an emergency message. Yielding.\n";

        //throw exception because gneE9_0 is Lane ID and not Route ID!
        //traciVehicle->changeRoute("gneE9_0", 60);

        traciVehicle->changeLane(0,yieldingTime);

//        //Try to make simulation run faster, but was not effective!!
//        cModule* appl = getParentModule()->getSubmodule("appl");
//        appl->par("dataOnSch") = false;

        //traciVehicle->setSpeed(0);
    }
    else {
            // Process legitimate WSM
        EV << "Received legitimate WSM with content: " << wsm->getDemoData() << "\n";
        findHost()->getDisplayString().setTagArg("i", 1, "green");
        if (mobility->getRoadId()[0] != ':') traciVehicle->changeRoute(wsm->getDemoData(), 9999);
        if (!sentMessage) {
            sentMessage = true;
        // repeat the received traffic update once in 2 seconds plus some random delay
            wsm->setSenderAddress(myId);
            wsm->setSerial(3);
            scheduleAt(simTime() + 2 + uniform(0.01, 0.2), wsm->dup());

        }
    }
}

void TraCIDemo11p::handleSelfMsg(cMessage* msg)
{
    if (TraCIDemo11pMessage* wsm = dynamic_cast<TraCIDemo11pMessage*>(msg)) {
        // send this message on the service channel until the counter is 3 or higher.
        // this code only runs when channel switching is enabled
        EV << "Inside first IF of TraCIDemo11p::handleSelfMsg \n";
        sendDown(wsm->dup());
        wsm->setSerial(wsm->getSerial() + 1);
        if (wsm->getSerial() >= 3) {
            EV << "Inside Second IF of TraCIDemo11p::handleSelfMsg \n";
            // stop service advertisements
            stopService();
            delete (wsm);
        }
        else {
            EV << "Inside first ELSE of TraCIDemo11p::handleSelfMsg \n";
            scheduleAt(simTime() + 1, wsm);
        }
    }
    else {
        EV << "Inside Second ELSE of TraCIDemo11p::handleSelfMsg \n";
        DemoBaseApplLayer::handleSelfMsg(msg);
    }
}

void TraCIDemo11p::handlePositionUpdate(cObject* obj)
{
    DemoBaseApplLayer::handlePositionUpdate(obj);

    // stopped for for at least 10s?
    if (mobility->getSpeed() < 1) {
        if (simTime() - lastDroveAt >= 10 && sentMessage == false) {
            findHost()->getDisplayString().setTagArg("i", 1, "red");
            sentMessage = true;

            TraCIDemo11pMessage* wsm = new TraCIDemo11pMessage();
            populateWSM(wsm);
            wsm->setDemoData(mobility->getRoadId().c_str());

            // host is standing still due to crash
            if (dataOnSch) {
                startService(Channel::sch2, 42, "Traffic Information Service");
                // started service and server advertising, schedule message to self to send later
                scheduleAt(computeAsynchronousSendingTime(1, ChannelType::service), wsm);
            }
            else {
                // send right away on CCH, because channel switching is disabled
                sendDown(wsm);
            }
        }
    }
    else {
        lastDroveAt = simTime();
    }
}
