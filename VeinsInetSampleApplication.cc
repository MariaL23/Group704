#include "veins_inet/VeinsInetSampleApplication.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/TagBase_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"
#include "inet/networklayer/common/InterfaceTable.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"

namespace veins {

using namespace inet;

Define_Module(VeinsInetSampleApplication);

VeinsInetSampleApplication::VeinsInetSampleApplication()
    : floodTimer(nullptr)
    , statsTimer(nullptr)
    , beaconTimer(nullptr)
    , compromised(false)
    , attackPacketsSent(0)
    , floodRate(0)
    , burstProbability(0)
    , burstSize(0)
    , packetSize(0)
    , initRetryCount(0)
    , lastProcessingTime(0)
{
}

VeinsInetSampleApplication::~VeinsInetSampleApplication() {
    cancelAndDelete(floodTimer);
    cancelAndDelete(statsTimer);
    cancelAndDelete(beaconTimer);
}

void VeinsInetSampleApplication::initialize(int stage) {
    VeinsInetApplicationBase::initialize(stage);

    if (stage == inet::INITSTAGE_LOCAL) {
        myId = getParentModule()->getFullName();
        floodRate = par("floodRate");
        burstProbability = par("burstProbability");
        burstSize = par("burstSize");
        packetSize = par("packetSize");
        beaconInterval = par("beaconInterval");
        beaconPacketSize = par("beaconPacketSize");

        // Register signals
        attackTrafficSignal = registerSignal("attackTraffic");
        burstCountSignal = registerSignal("burstCount");
        packetRateSignal = registerSignal("packetRate");
        queueLengthSignal = registerSignal("queueLength");
        processingDelaySignal = registerSignal("processingDelay");
        bytesReceivedSignal = registerSignal("bytesReceived");
        beaconsSentSignal = registerSignal("beaconsSent");

        // Initialize timers
        statsTimer = new cMessage("statsTimer");
        beaconTimer = new cMessage("beaconTimer");

        // Schedule initial beacon timer
        scheduleAt(simTime() + uniform(0, beaconInterval), beaconTimer);
        scheduleAt(simTime() + 1.0, statsTimer);
    }
    else if (stage == inet::INITSTAGE_NETWORK_LAYER) {
        auto initTimer = new cMessage("initializeAddress");
        scheduleAt(simTime() + 0.1, initTimer);
    }
}

void VeinsInetSampleApplication::initializeAddress() {
    auto ift = getModuleByPath("^.interfaceTable");
    if (!ift) {
        EV_ERROR << myId << ": Cannot find interface table" << endl;
        scheduleInitRetry();
        return;
    }

    auto interfaceTable = check_and_cast<inet::IInterfaceTable*>(ift);
    auto interface = interfaceTable->findInterfaceByName("wlan0");

    if (!interface) {
        EV_ERROR << myId << ": Cannot find wlan0 interface" << endl;
        scheduleInitRetry();
        return;
    }

    auto ipv4Data = interface->getProtocolData<inet::Ipv4InterfaceData>();
    if (!ipv4Data) {
        EV_ERROR << myId << ": No IPv4 data on interface" << endl;
        scheduleInitRetry();
        return;
    }

    if (ipv4Data->getIPAddress().isUnspecified()) {
        EV_WARN << myId << ": IP address not yet configured" << endl;
        scheduleInitRetry();
        return;
    }

    ownAddress = ipv4Data->getIPAddress();
    EV_INFO << myId << ": Successfully initialized IP address to " << ownAddress << endl;
}

void VeinsInetSampleApplication::scheduleInitRetry() {
    initRetryCount++;
    auto timer = new cMessage("initializeAddress");
    scheduleAt(simTime() + 2.0 * pow(2, initRetryCount), timer);
}

void VeinsInetSampleApplication::handleMessageWhenUp(cMessage* msg) {
    if (msg->isSelfMessage()) {
        if (strcmp(msg->getName(), "initializeAddress") == 0) {
            delete msg;
            initializeAddress();
        }
        else if (msg == statsTimer) {
            updateStatistics();
            scheduleAt(simTime() + 1.0, statsTimer);
        }
        else if (msg == beaconTimer) {
            sendBeacon();
            scheduleAt(simTime() + beaconInterval, beaconTimer);
        }
        else if (msg == floodTimer && compromised) {
            if (uniform(0, 1) < burstProbability) {
                sendFloodPackets(burstSize);
                emit(burstCountSignal, 1);
                scheduleAt(simTime() + exponential(0.05), floodTimer);
            } else {
                sendFloodPackets(floodRate);
                scheduleAt(simTime() + exponential(0.1), floodTimer);
            }
        }
    } else {
        VeinsInetApplicationBase::handleMessageWhenUp(msg);
    }
}

int VeinsInetSampleApplication::numInitStages() const {
    return inet::NUM_INIT_STAGES;
}

void VeinsInetSampleApplication::sendBeacon() {
    auto payload = inet::makeShared<inet::ByteCountChunk>(inet::B(beaconPacketSize));
    auto packet = createPacket("V2VBeacon");
    packet->insertAtBack(payload);
    sendPacket(std::move(packet));  // Use base class sendPacket for multicast
    emit(beaconsSentSignal, 1);
}

void VeinsInetSampleApplication::processPacket(std::shared_ptr<inet::Packet> pk) {
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
}

void VeinsInetSampleApplication::sendDDoSPacket() {
    if (!compromised || victimAddress.isUnspecified()) {
        return;
    }

    auto packet = new Packet("DDoSAttack");
    auto payload = inet::makeShared<inet::ByteCountChunk>(inet::B(packetSize));
    packet->insertAtBack(payload);

    socket.sendTo(packet, victimAddress, portNumber);
    emit(attackTrafficSignal, 1);
}

void VeinsInetSampleApplication::sendFloodPackets(int numPackets) {
    for (int i = 0; i < numPackets; i++) {
        sendDDoSPacket();
    }
    emit(attackTrafficSignal, numPackets);
}

void VeinsInetSampleApplication::updateStatistics() {
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

bool VeinsInetSampleApplication::startApplication() {
    return true;
}

bool VeinsInetSampleApplication::stopApplication() {
    if (floodTimer) {
        cancelAndDelete(floodTimer);
        floodTimer = nullptr;
    }
    return true;
}

}
