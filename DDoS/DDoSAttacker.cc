#include "veins_inet/DDoSAttacker.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/transportlayer/common/L4PortTag_m.h"


namespace veins {
using namespace inet;

Define_Module(DDoSAttacker);

DDoSAttacker::DDoSAttacker()
    : floodTimer(nullptr)
    , compromiseTimer(nullptr)
    , beaconTimer(nullptr)
    , attackActive(false)
    , attackPacketsSent(0)
    , compromisePacketsSent(0)
{
}

DDoSAttacker::~DDoSAttacker() {
    cancelAndDelete(floodTimer);
    cancelAndDelete(compromiseTimer);
    cancelAndDelete(beaconTimer);
}

void DDoSAttacker::resolveVictimAddress() {
    std::string victimNodeName = getParentModule()->getParentModule()->getFullName() +
                               std::string(".node[") + victimId + "]";
    try {
        victimAddress = inet::L3AddressResolver().resolve(victimNodeName.c_str());
        EV_INFO << "Resolved victim address: " << victimAddress << endl;
    }
    catch (const std::exception& e) {
        EV_ERROR << "Failed to resolve victim address: " << e.what() << endl;
    }
}

bool DDoSAttacker::startApplication() {
    // Initialize parameters
    floodRate = par("floodRate");
    burstProbability = par("burstProbability");
    burstSize = par("burstSize");
    packetSize = par("packetSize");
    victimId = par("victimId").stdstringValue();
    compromiseInterval = par("compromiseInterval");
    beaconInterval = par("beaconInterval");
    beaconPacketSize = par("beaconPacketSize");

    // Get IP address
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

    // Resolve victim address
    resolveVictimAddress();

    // Register signals
    attackTrafficSignal = registerSignal("attackTraffic");
    burstCountSignal = registerSignal("burstCount");
    compromiseAttemptsSignal = registerSignal("compromiseAttempts");
    beaconsSentSignal = registerSignal("beaconsSent");

    // Initialize timers
    floodTimer = new cMessage("floodTimer");
    compromiseTimer = new cMessage("compromiseTimer");
    beaconTimer = new cMessage("beaconTimer");

    // Schedule timers
    scheduleAt(simTime() + 30.5, compromiseTimer);  // Attack starts at 30.5s
    scheduleAt(simTime() + 31.0, floodTimer);      // Attack continues at 31.0s
    scheduleAt(simTime() + uniform(0, beaconInterval), beaconTimer);  // Start beacons immediately

    attackActive = true;
    return true;
}

void DDoSAttacker::sendBeacon() {
    auto payload = inet::makeShared<inet::ByteCountChunk>(inet::B(beaconPacketSize));
    auto packet = createPacket("V2VBeacon");
    packet->insertAtBack(payload);
    sendPacket(std::move(packet));  // Use base class sendPacket for multicast
    emit(beaconsSentSignal, 1);
}

void DDoSAttacker::handleMessageWhenUp(cMessage* msg) {
    if (msg->isSelfMessage()) {
        if (msg == beaconTimer) {
            sendBeacon();
            scheduleAt(simTime() + beaconInterval, beaconTimer);
        }
        else if (msg == floodTimer || msg == compromiseTimer) {
            handleSelfMessage(msg);
        }
    } else {
        VeinsInetApplicationBase::handleMessageWhenUp(msg);
    }
}

void DDoSAttacker::sendDDoSPacket(bool isCompromise) {
    if (!isCompromise) {
        if (victimAddress.isUnspecified()) {
            resolveVictimAddress();
            if (victimAddress.isUnspecified()) {
                EV_ERROR << "Cannot send attack packet: victim address not resolved" << endl;
                return;
            }
        }

        // Create attack packet for unicast
        auto packet = new Packet("DDoSAttack");
        auto payload = inet::makeShared<inet::ByteCountChunk>(inet::B(packetSize));
        packet->insertAtBack(payload);

        socket.sendTo(packet, victimAddress, portNumber);
        emit(attackTrafficSignal, 1);
    }
    else {
        // Create compromise packet for multicast
        auto payload = inet::makeShared<inet::ByteCountChunk>(inet::B(packetSize));
        auto packet = createPacket("CompromiseCommand");
        packet->insertAtBack(payload);
        sendPacket(std::move(packet));
    }
}

void DDoSAttacker::sendFloodPackets(int numPackets) {
    for (int i = 0; i < numPackets; i++) {
        sendDDoSPacket(false);
    }
    emit(attackTrafficSignal, numPackets);
}

void DDoSAttacker::sendCompromisePacket() {
    sendDDoSPacket(true);
    compromisePacketsSent++;
    emit(compromiseAttemptsSignal, 1);
}

void DDoSAttacker::handleSelfMessage(cMessage* msg) {
    if (msg == floodTimer && attackActive) {
        if (uniform(0, 1) < burstProbability) {
            sendFloodPackets(burstSize);
            emit(burstCountSignal, 1);
        } else {
            sendFloodPackets(floodRate);
        }
        scheduleAt(simTime() + exponential(0.1), floodTimer);
    }
    else if (msg == compromiseTimer) {
        sendCompromisePacket();
        scheduleAt(simTime() + compromiseInterval, compromiseTimer);
    }
}

void DDoSAttacker::processPacket(std::shared_ptr<inet::Packet> pk) {
    EV_INFO << "Received packet - ignoring as we are the attacker" << endl;
}

bool DDoSAttacker::stopApplication() {
    attackActive = false;
    return true;
}

} // namespace veins
