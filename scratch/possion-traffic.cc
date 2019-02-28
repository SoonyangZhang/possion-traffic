#include<iostream>
#include<stdlib.h>
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
static double simDuration=100;
uint16_t client_port=1234;
uint16_t serv_port=4321;
float appStart=0.0;
float appStop=simDuration-1;
int main(int argc, char *argv[]){
	LogComponentEnable("PossionSender",LOG_LEVEL_ALL);
	LogComponentEnable("PossionReceiver",LOG_LEVEL_ALL);
	uint64_t linkBw_1   = 1000000;//4000000;
    uint32_t msDelay_1  = 100;//50;//100;
    uint32_t msQDelay_1 = 200;
    NodeContainer nodes = BuildExampleTopo (linkBw_1, msDelay_1, msQDelay_1);
    Ptr<PossionSender> ps_app=CreateObject<PossionSender>(1000000);
    Ptr<PossionReceiver> pr_app=CreateObject<PossionReceiver>();
    nodes.Get(0)->AddApplication (ps_app);
    nodes.Get(1)->AddApplication (pr_app);
    ps_app->Bind(client_port);
    pr_app->Bind(serv_port);
    ps_app->SetStartTime (Seconds (appStart));
    ps_app->SetStopTime (Seconds (appStop));
    pr_app->SetStartTime (Seconds (appStart));
    pr_app->SetStopTime (Seconds (appStop));
    InetSocketAddress remote=pr_app->GetLocalAddress();
   	ps_app->ConfigurePeer(remote.GetIpv4(),remote.GetPort());
   	
   	PossionTrace trace;
   	std::string log="posssion";
   	trace.OpenTraceOwdFile(log);
   	trace.OpenTraceRttFile(log);
   	trace.OpenTraceSendGapFile(log);
   	ps_app->SetTraceRttFun(MakeCallback(&PossionTrace::OnRtt,&trace));
   	ps_app->SetTraceGapFun(MakeCallback(&PossionTrace::OnGap,&trace));
   	pr_app->SetOwdTraceFuc(MakeCallback(&PossionTrace::OnOwd,&trace));
   	Simulator::Stop (Seconds(simDuration));
    Simulator::Run ();
    Simulator::Destroy();
    printf("stop");
	return 0;
}
