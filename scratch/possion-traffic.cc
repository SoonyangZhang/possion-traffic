#include<iostream>
#include<stdlib.h>
#include <memory>
#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/traffic-control-module.h"
#include "ns3/simulator.h"
#include "ns3/possionsender.h"
#include "ns3/possionreceiver.h"
#include "ns3/possiontrace.h"
#include "ns3/log.h"
using namespace std;
using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("Mock-Test");
const uint32_t DEFAULT_PACKET_SIZE = 1000;
static NodeContainer BuildExampleTopo (uint64_t bps,
                                       uint32_t msDelay,
                                       uint32_t msQdelay)
{
    NodeContainer nodes;
    nodes.Create (2);

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute ("DataRate", DataRateValue  (DataRate (bps)));
    pointToPoint.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (msDelay)));
    auto bufSize = std::max<uint32_t> (DEFAULT_PACKET_SIZE, bps * msQdelay / 8000);
    pointToPoint.SetQueue ("ns3::DropTailQueue",
                           "Mode", StringValue ("QUEUE_MODE_BYTES"),
                           "MaxBytes", UintegerValue (bufSize));
    NetDeviceContainer devices = pointToPoint.Install (nodes);

    InternetStackHelper stack;
    stack.Install (nodes);
    Ipv4AddressHelper address;
    std::string nodeip="10.1.1.0";
    address.SetBase (nodeip.c_str(), "255.255.255.0");
    address.Assign (devices);

    // Uncomment to capture simulated traffic
    // pointToPoint.EnablePcapAll ("rmcat-example");

    // disable tc for now, some bug in ns3 causes extra delay
    TrafficControlHelper tch;
    tch.Uninstall (devices);
/*
	std::string errorModelType = "ns3::RateErrorModel";
  	ObjectFactory factory;
  	factory.SetTypeId (errorModelType);
  	Ptr<ErrorModel> em = factory.Create<ErrorModel> ();
	devices.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));*/
    return nodes;
}
static void InstallPossionApplication(
                         Ptr<Node> sender,
                         Ptr<Node> receiver,
						 uint16_t send_port,
                         uint16_t recv_port,uint32_t bps,
                         float startTime,
                         float stopTime,
						 PossionTrace *trace
)
{
    Ptr<PossionSender> sendApp = CreateObject<PossionSender> (bps);
	Ptr<PossionReceiver> recvApp = CreateObject<PossionReceiver>();
   	sender->AddApplication (sendApp);
    receiver->AddApplication (recvApp);
    Ptr<Ipv4> ipv4 = receiver->GetObject<Ipv4> ();
	Ipv4Address receiverIp = ipv4->GetAddress (1, 0).GetLocal();
	recvApp->Bind(recv_port);
	sendApp->Bind(send_port);
	sendApp->ConfigurePeer(receiverIp,recv_port);
    sendApp->SetStartTime (Seconds (startTime));
    sendApp->SetStopTime (Seconds (stopTime));
    recvApp->SetStartTime (Seconds (startTime));
    recvApp->SetStopTime (Seconds (stopTime));
	if(trace){
        sendApp->SetTraceRttFun(MakeCallback(&PossionTrace::OnRtt,trace));
        sendApp->SetTraceGapFun(MakeCallback(&PossionTrace::OnGap,trace));
        recvApp->SetOwdTraceFuc(MakeCallback(&PossionTrace::OnOwd,trace));
	}	
}
static double simDuration=200;
float appStart=0.0;
float appStop=simDuration-1;
int main(int argc, char *argv[]){
	LogComponentEnable("PossionSender",LOG_LEVEL_ALL);
	LogComponentEnable("PossionReceiver",LOG_LEVEL_ALL);
	uint64_t linkBw   = 3000000;//4000000;
    uint32_t msDelay  = 100;//50;//100;
    uint32_t msQDelay = 200;
    uint32_t num_clients=3;
    uint32_t num_log=num_clients;
    uint32_t send_port=1234;
    uint32_t recv_port=send_port+num_clients;
    NodeContainer nodes = BuildExampleTopo (linkBw, msDelay, msQDelay);
    std::list<std::shared_ptr<PossionTrace>> traces;
    uint32_t bps=1000000;
    uint32_t i=0;
    for(i=0;i<num_clients;i++){
        std::string name="possion_"+std::to_string(i+1);
        if(i<num_log){
            std::shared_ptr<PossionTrace> trace(new PossionTrace());
            trace->Log(name,E_POSSION_OWD);
            InstallPossionApplication( nodes.Get(0), nodes.Get(1),send_port,recv_port,bps,
            appStart,appStop,trace.get());
            traces.push_back(trace);
        }else{
            InstallPossionApplication( nodes.Get(0), nodes.Get(1),send_port,recv_port,bps,
            appStart,appStop,nullptr);
        }
        send_port++;
	recv_port++;
        
    }
   	Simulator::Stop (Seconds(simDuration));
    Simulator::Run ();
    Simulator::Destroy();
    while(!traces.empty()){
        traces.pop_front();
    }
    printf("stop");
	return 0;
}
