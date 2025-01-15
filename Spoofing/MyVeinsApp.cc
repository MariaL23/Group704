#include "veins/modules/application/traci/MyVeinsApp.h"
#include "TraCIDemo11pMessage_m.h"

using namespace veins;

Define_Module(veins::MyVeinsApp);

void MyVeinsApp::initialize(int stage)
{

    EV_INFO << "MyVeinsApp::initialize\n";

    DemoBaseApplLayer::initialize(stage);
    if (stage == 0) {

        cModule* nic = getParentModule()->getSubmodule("nic")->getSubmodule("mac1609_4");
        nic->par("useServiceChannel") = true;

        cModule* appl = getParentModule()->getSubmodule("appl");
        appl->par("dataOnSch") = true;

        broadcastMessageEvent = new cMessage("BroadcastMessageEvent");
        scheduleAt(simTime() + broadcastInterval, broadcastMessageEvent);



// Drive in Passing Lane
        traciVehicle->changeLane(1,2000.0);
    }
    else if (stage == 1) {
        // Initializing members that require initialized other modules goes here
    }
}

void MyVeinsApp::finish()
{
    cancelAndDelete(broadcastMessageEvent);
    DemoBaseApplLayer::finish();
    // statistics recording goes here
}

void MyVeinsApp::onBSM(DemoSafetyMessage* bsm)
{
    // Your application has received a beacon message from another car or RSU
    // code for handling the message goes here
}

void MyVeinsApp::onWSM(BaseFrame1609_4* wsm)
{
    // Your application has received a data message from another car or RSU
    // code for handling the message goes here, see TraciDemo11p.cc for examples
}

void MyVeinsApp::onWSA(DemoServiceAdvertisment* wsa)
{
    // Your application has received a service advertisement from another car or RSU
    // code for handling the message goes here, see TraciDemo11p.cc for examples
}

void MyVeinsApp::handleSelfMsg(cMessage* msg)
{
    EV_INFO << "MyVeinsApp::handleSelfMsg.\n";

    if (msg == broadcastMessageEvent) {
        sendEmergencyMessage(); // Send the "emergency" broadcast
        // Reschedule the broadcast event to occur every <broadcastInterval> seconds
        scheduleAt(simTime() + broadcastInterval, broadcastMessageEvent);
        } else {
            DemoBaseApplLayer::handleSelfMsg(msg);
    // this method is for self messages (mostly timers)
    // it is important to call the DemoBaseApplLayer function for BSM and WSM transmission
        }
}

void MyVeinsApp::sendEmergencyMessage() {
    // Create a new "emergency" message
    TraCIDemo11pMessage*  emergencyMessage = new TraCIDemo11pMessage("EmergencyBroadcast");
    emergencyMessage->setKind(1); // Set a unique kind for easy recognition

    //    AttackerPosition = mobility->getCurrentDirection();
    //    emergencyMessage->addPar("X").setDoubleValue(AttackerPosition .x);
    //    emergencyMessage->addPar("Y").setDoubleValue(AttackerPosition .y);


    emergencyMessage->setByteLength(100);
    sendDown(emergencyMessage); // Send the message down to be broadcast

    EV_INFO << "Attacker sent an emergency message.\n";


//    //Try to make simulation run faster, but was not effective!!
//    cModule* nic = getParentModule()->getSubmodule("nic")->getSubmodule("mac1609_4");
//            // Reset `useServiceChannel` to false
//    nic->par("useServiceChannel") = false;
//
//    cModule* appl = getParentModule()->getSubmodule("appl");
//    appl->par("dataOnSch") = false;

}

void MyVeinsApp::handlePositionUpdate(cObject* obj)
{

    DemoBaseApplLayer::handlePositionUpdate(obj);

    // the vehicle has moved. Code that reacts to new positions goes here.
    // member variables such as currentPosition and currentSpeed are updated in the parent class
}
