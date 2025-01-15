#include "veins/modules/application/traci/MITMAttackModule.h"

using namespace veins;

Define_Module(veins::MITMAttackNode);


MITMAttackNode::~MITMAttackNode() {
    cancelAndDelete(attackEvent);
}

void MITMAttackNode::initialize(int stage) {
    DemoBaseApplLayer::initialize(stage);

    if (stage == 0) {
           // Initialize attack parameters
           attackRadius = 500.0;
           lastDroveAt = simTime();
           sentMessage = false;

           // Get mobility module
           mobility = TraCIMobilityAccess().get(getParentModule());
           if (!mobility) {
               throw cRuntimeError("Could not find mobility module");
           }

           // Get TraCI interface
           traci = mobility->getCommandInterface();
           if (!traci) {
               throw cRuntimeError("Could not get TraCI command interface");
           }

           // Create attack event
           attackEvent = new cMessage("attackTrigger");

           // Schedule first attack to coincide with accident time
           scheduleAt(simTime() + 73, attackEvent);  // Start at 73s

           EV << "MITMAttackNode initialized - will start attack at 73s" << endl;
           EV << "Initial position: " << mobility->getPositionAt(simTime()).info() << endl;
       }
}

void MITMAttackNode::handleLowerMsg(cMessage* msg) {
    BaseFrame1609_4* wsm = dynamic_cast<BaseFrame1609_4*>(msg);
    if (wsm) {
        onWSM(wsm);
    }
    delete msg;
}

void MITMAttackNode::handleSelfMsg(cMessage* msg) {
    if (msg == attackEvent) {
        // Create and send attack message
        TraCIDemo11pMessage* attackMsg = new TraCIDemo11pMessage("attackMsg");
        populateWSM(attackMsg);

        // Set attack message parameters
        attackMsg->setDemoData("STOP_COMMAND");
        attackMsg->addPar("isTampered").setBoolValue(true);
        attackMsg->addPar("attackerId").setStringValue(mobility->getExternalId().c_str());
        attackMsg->addPar("command").setStringValue("stop");

        // Send attack message
        sendDown(attackMsg);

        // Schedule next attack
        scheduleAt(simTime() + 1, attackEvent->dup());

        EV << "Periodic attack message sent at " << simTime() << endl;
    }
    delete msg;
}

void MITMAttackNode::onWSM(BaseFrame1609_4* wsm) {
    EV << "MITMAttackNode received message at " << simTime() << endl;

    // Create attack message
    TraCIDemo11pMessage* attackMsg = new TraCIDemo11pMessage("attackMsg");
    populateWSM(attackMsg);

    // Set attack message parameters
    attackMsg->setDemoData("STOP_COMMAND");
    attackMsg->addPar("isTampered").setBoolValue(true);
    attackMsg->addPar("attackerId").setStringValue(mobility->getExternalId().c_str());
    attackMsg->addPar("command").setStringValue("stop");

    // Send attack message
    sendDown(attackMsg);
    EV << "Attack message sent at " << simTime() << endl;
}

void MITMAttackNode::launchAttack() {
    // Create and send attack message
    TraCIDemo11pMessage* attackMsg = new TraCIDemo11pMessage("attackMsg");
    populateWSM(attackMsg);
    attackMsg->setDemoData("STOP_COMMAND");
    attackMsg->addPar("isTampered").setBoolValue(true);
    attackMsg->addPar("attackerId").setStringValue(mobility->getExternalId().c_str());

    // Broadcast attack message
    sendDown(attackMsg);
    EV << "Active attack launched at " << simTime() << endl;
}

void MITMAttackNode::handlePositionUpdate(cObject* obj) {
    DemoBaseApplLayer::handlePositionUpdate(obj);

    // Update attacker's visual representation
    findHost()->getDisplayString().setTagArg("i", 1, "red");
}

void MITMAttackNode::finish() {
    DemoBaseApplLayer::finish();
}
