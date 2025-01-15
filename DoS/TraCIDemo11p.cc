#include "veins/modules/application/traci/TraCIDemo11p.h"
#include "veins/modules/application/traci/TraCIDemo11pMessage_m.h"

using namespace veins;

Define_Module(veins::TraCIDemo11p);

void TraCIDemo11p::initialize(int stage)
{
    DemoBaseApplLayer::initialize(stage);
    if (stage == 0) {
        sentMessage = false;
        lastDroveAt = simTime();
        currentSubscribedServiceId = -1;

        // Register latency signal
        latencySignal = registerSignal("latencySignal");

        // Initialize new tracking variables
        totalBytesSent = 0;
        bandwidthStartTime = 0;
        totalPacketsSent = 0;
        totalPacketsReceived = 0;
        totalLatency = 0.0;
        latencyCount = 0;
        maxLatency = 0.0;
    }
}

void TraCIDemo11p::onWSA(DemoServiceAdvertisment* wsa)
{
    if (currentSubscribedServiceId == -1) {
        mac->changeServiceChannel(static_cast<Channel>(wsa->getTargetChannel()));
        currentSubscribedServiceId = wsa->getPsid();
        EV << "WSA received" << "\n";
        if (currentOfferedServiceId != wsa->getPsid()) {
            stopService();
            startService(static_cast<Channel>(wsa->getTargetChannel()), wsa->getPsid(), "Mirrored Traffic Service");
        }
    }
}

void TraCIDemo11p::onWSM(BaseFrame1609_4* frame)
{
    TraCIDemo11pMessage* wsm = check_and_cast<TraCIDemo11pMessage*>(frame);

    // Track received packets
    totalPacketsReceived++;

    findHost()->getDisplayString().setTagArg("i", 1, "green");

    // Validate timestamp and calculate latency
    if (wsm->getTimestamp() > 0) {
        simtime_t latency = simTime() - wsm->getTimestamp();
        EV << "Latency: " << latency << " seconds\n";

        // Emit latency signal
        emit(latencySignal, latency.dbl());
        totalLatency += latency.dbl();
        latencyCount++;

        // Update max latency
        if (latency.dbl() > maxLatency) {
            maxLatency = latency.dbl();
        }
    } else {
        EV << "Received WSM with invalid timestamp, skipping latency calculation.\n";
    }

    if (!sentMessage) {
        sentMessage = true;
        if (getParentModule()->getIndex() == 100) { // change this to change attacker node
            handleNode1Flooding();
        }
    } else {
        wsm->setSenderAddress(getParentModule()->getIndex());
        totalPacketsSent++;
        scheduleAt(simTime() + 0 + uniform(0.01, 0.01), wsm->dup());
    }
}

void TraCIDemo11p::handleSelfMsg(cMessage* msg)
{
    if (TraCIDemo11pMessage* wsm = dynamic_cast<TraCIDemo11pMessage*>(msg)) {
        // Add timestamp to the WSM
        wsm->setTimestamp(simTime());

        // Set payload size for node 1
        if (getParentModule()->getIndex() == 100) {
            // Custom payload size logic for node 1 (if needed)
        } else {
            int payloadSize = wsm->getDemoData() ? strlen(wsm->getDemoData()) : 100;
            wsm->setByteLength(payloadSize + 50); // Default size for other nodes
        }

        totalBytesSent += wsm->getByteLength();

        sendDown(wsm->dup());
        scheduleAt(simTime() + uniform(1, 3), new TraCIDemo11pMessage());
        EV << "Node[" << getParentModule()->getIndex() << "] is sending down WSM with byte length: " << wsm->getByteLength() << "\n";
    } else {
        DemoBaseApplLayer::handleSelfMsg(msg);
    }
}

void TraCIDemo11p::handlePositionUpdate(cObject* obj)
{
    DemoBaseApplLayer::handlePositionUpdate(obj);

    // stopped for at least 10s?
    if (mobility->getSpeed() < 1) {
        if (simTime() - lastDroveAt >= 10 && sentMessage == false) {
            findHost()->getDisplayString().setTagArg("i", 1, "red");
            sentMessage = true;

            TraCIDemo11pMessage* wsm = new TraCIDemo11pMessage();
            populateWSM(wsm);

            // Add timestamp to the WSM
            wsm->setTimestamp(simTime());

            wsm->setDemoData(mobility->getRoadId().c_str());

            if (dataOnSch) {
                startService(Channel::sch2, 42, "Traffic Information Service");
                scheduleAt(computeAsynchronousSendingTime(1, ChannelType::service), wsm);
            } else {
                sendDown(wsm);
            }
        }
    } else {
        lastDroveAt = simTime();
    }
}

void TraCIDemo11p::handleNode1Flooding()
{
    // Record start time for bandwidth calculation
    bandwidthStartTime = simTime();

    // Number of messages
    int floodMessageCount = 40000;

    // Track total packets for flooding
    totalPacketsSent += floodMessageCount;

    // Intervals and message characteristics
    for (int i = 0; i < floodMessageCount; i++) {
        TraCIDemo11pMessage* floodWsm = new TraCIDemo11pMessage();

        // Randomize sender address to simulate multiple attackers
        floodWsm->setSenderAddress(intuniform(0, getParentModule()->getParentModule()->getSubmodule("node", 0)->getVectorSize() - 1));

        floodWsm->setSerial(i);

        // Message payload size
        int payloadSize = 100000;

        floodWsm->setByteLength(payloadSize);

        double sendInterval = 0.01;
        scheduleAt(simTime() + i * sendInterval, floodWsm);

        wsmSentCount++;
    }

    EV << "Advanced Flooding Attack: " << floodMessageCount << " messages" << "\n";
}

void TraCIDemo11p::finish()
{
    // Record basic sending metrics
    recordScalar("TotalWSMSent", wsmSentCount);
    recordScalar("TotalBytesSent", totalBytesSent);

    // Calculate bandwidth consumption
    double bandwidth = totalBytesSent / (simTime() - bandwidthStartTime).dbl();
    recordScalar("BandwidthConsumption", bandwidth); // bytes per second
    EV << "Final Counts - Sent: " << totalPacketsSent << ", Received: " << totalPacketsReceived << "\n";

    // Packet loss calculation
    double packetLossRate = 1.0 - (double)totalPacketsReceived / totalPacketsSent;
    recordScalar("TotalPacketsSent", totalPacketsSent);
    recordScalar("TotalPacketsReceived", totalPacketsReceived);
    recordScalar("PacketLossRate", packetLossRate);

    // Latency calculation
    if (latencyCount > 0) {
        double averageLatency = totalLatency / latencyCount;
        recordScalar("AverageLatency", averageLatency);
        recordScalar("MaxLatency", maxLatency);
        EV << "Latency Metrics - Average: " << averageLatency << " seconds, Max: " << maxLatency << " seconds\n";
    } else {
        EV << "No valid latency data collected.\n";
    }
}
