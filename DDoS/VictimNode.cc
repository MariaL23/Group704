#include "veins_inet/VictimNode.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"

namespace veins {
using namespace inet;

Define_Module(VictimNode);

VictimNode::VictimNode()
    : statsTimer(nullptr)
    , beaconTimer(nullptr)
    , attackPacketsSent(0)
    , lastProcessingTime(0)
{
}

VictimNode::~VictimNode() {
    cancelAndDelete(statsTimer);
    cancelAndDelete(beaconTimer);
}

void VictimNode::initialize(int stage) {
    VeinsInetApplicationBase::initialize(stage);

    if (stage == inet::INITSTAGE_LOCAL) {
        // Get parameters
        beaconInterval = par("beaconInterval");
        beaconPacketSize = par("beaconPacketSize");

        // Register signals
        packetRateSignal = registerSignal("packetRate");
        queueLengthSignal = registerSignal("queueLength");
        processingDelaySignal = registerSignal("processingDelay");
        bytesReceivedSignal = registerSignal("bytesReceived");
        beaconsSentSignal = registerSignal("beaconsSent");

        // Initialize timers
        statsTimer = new cMessage("statsTimer");
        beaconTimer = new cMessage("beaconTimer");

        // Schedule timers
        scheduleAt(simTime() + 1.0, statsTimer);
        scheduleAt(simTime() + uniform(0, beaconInterval), beaconTimer);

        // Get own IP address
        auto ift = getModuleByPath("^.interfaceTable");
        if (ift) {
            auto interfaceTable = check_and_cast<inet::IInterfaceTable*>(ift);
            auto interface = interfaceTable->findInterfaceByName("wlan0");
            if (interface) {
                auto ipv4Data = interface->getProtocolData<inet::Ipv4InterfaceData>();
                if (ipv4Data) {
                    ownAddress = ipv4Data->getIPAddress();
                }
            }
        }
    }
}

void VictimNode::sendBeacon() {
    auto payload = inet::makeShared<inet::ByteCountChunk>(inet::B(beaconPacketSize));
    auto packet = createPacket("V2VBeacon");
    packet->insertAtBack(payload);
    sendPacket(std::move(packet));  // Use base class sendPacket for multicast
    emit(beaconsSentSignal, 1);
}

void VictimNode::handleMessageWhenUp(cMessage* msg) {
    if (msg->isSelfMessage()) {
        if (msg == beaconTimer) {
            sendBeacon();
            scheduleAt(simTime() + beaconInterval, beaconTimer);
        }
        else if (msg == statsTimer) {
            updateStatistics();
            scheduleAt(simTime() + 1.0, statsTimer);
        }
    } else {
        VeinsInetApplicationBase::handleMessageWhenUp(msg);
    }
}
void VictimNode::handleSelfMessage(omnetpp::cMessage* msg) {
    if (msg == beaconTimer) {
        sendBeacon();
        scheduleAt(simTime() + beaconInterval, beaconTimer);
    }
    else if (msg == statsTimer) {
        updateStatistics();
        scheduleAt(simTime() + 1.0, statsTimer);
    }
}

void VictimNode::processPacket(std::shared_ptr<inet::Packet> pk) {
    EV_DETAIL << "Victim received packet: " << pk->getName()
              << " from " << pk->getTag<inet::L3AddressInd>()->getSrcAddress()
              << " size: " << pk->getByteLength() << " bytes"
              << " at time: " << simTime() << endl;

    // Update packet statistics
    packetCounter[simTime().dbl()]++;
    processingQueue.push(pk);

    // Emit statistics
    emit(queueLengthSignal, (long)processingQueue.size());
    emit(bytesReceivedSignal, (long)pk->getByteLength());

    // Calculate and emit processing delay
    simtime_t processingDelay = simTime() - lastProcessingTime;
    emit(processingDelaySignal, processingDelay);
    lastProcessingTime = simTime();

    // Visual indication of being under attack
    getParentModule()->getDisplayString().setTagArg("i", 1, "red");

    // Log attack source
    auto srcAddr = pk->getTag<inet::L3AddressInd>()->getSrcAddress();
    EV_INFO << "Victim node received packet from " << srcAddr << endl;
}

void VictimNode::updateStatistics() {
    int packetsLastSecond = 0;
    simtime_t currentTime = simTime();

    for (auto it = packetCounter.begin(); it != packetCounter.end();) {
        if (currentTime - it->first <= 1.0) {
            packetsLastSecond += it->second;
            ++it;
        } else {
            it = packetCounter.erase(it);
        }
    }

    emit(packetRateSignal, packetsLastSecond);
}

bool VictimNode::startApplication() {
    getParentModule()->getDisplayString().setTagArg("i", 1, "blue");
    return true;
}


bool VictimNode::stopApplication() {
    while (!processingQueue.empty()) {
        processingQueue.pop();
    }
    packetCounter.clear();
    return true;
}

} // namespace veins
