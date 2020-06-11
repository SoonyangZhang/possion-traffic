#include "ns3/possionreceiver.h"
#include "ns3/log.h"
#include <memory.h>
#include "byte_order.h"
#include "mock_proto.h"
namespace ns3{
NS_LOG_COMPONENT_DEFINE("PossionReceiver");
void PossionReceiver::Bind(uint16_t port){
    if (m_socket== NULL) {
        m_socket = Socket::CreateSocket (GetNode (),UdpSocketFactory::GetTypeId ());
        auto local = InetSocketAddress{Ipv4Address::GetAny (), port};
        auto res = m_socket->Bind (local);
        NS_ASSERT (res == 0);
    }
    m_bindPort=port;
    m_socket->SetRecvCallback (MakeCallback(&PossionReceiver::RecvPacket,this));
}
InetSocketAddress PossionReceiver::GetLocalAddress(){
    Ptr<Node> node=GetNode();
    Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
    Ipv4Address local_ip = ipv4->GetAddress (1, 0).GetLocal ();
	return InetSocketAddress{local_ip,m_bindPort};
}
void PossionReceiver::StartApplication(){}
void PossionReceiver::StopApplication(){
	m_running=false;
}
void PossionReceiver::RecvPacket(Ptr<Socket> socket){
	if(!m_running){
		return ;
	}
	Address remoteAddr;
	auto packet = socket->RecvFrom (remoteAddr);
	if(!m_knowPeer){
        m_peerIp= InetSocketAddress::ConvertFrom (remoteAddr).GetIpv4 ();
	    m_peerPort= InetSocketAddress::ConvertFrom (remoteAddr).GetPort ();
		m_knowPeer=true;
	}
    MockHeader header;
    packet->RemoveHeader(header);
    if(header.GetFrameType()==MockHeader::STREAM){
        MockPacketNumber seq=header.GetFrameSequence();
        uint32_t sent_ts=header.GetTimeStamp();
        if(!m_largest_receipt.IsInitialized()||seq>m_largest_receipt){
            m_largest_receipt=seq;
            CreateAck(sent_ts);
        }
    }
}
void PossionReceiver::CreateAck(uint32_t sent_ts){
	uint32_t receive_time=Simulator::Now().GetMilliSeconds();
	uint32_t owd=receive_time-sent_ts;
	if(!m_traceOwdCb.IsNull()){
		m_traceOwdCb((uint32_t)m_largest_receipt.ToUint64(),owd);
	}
    MockHeader header(m_largest_receipt,receive_time,MockHeader::ACK);
	Ptr<Packet> p=Create<Packet>(header.GetSerializedSize());
    p->AddHeader(header);
	SendToNetwork(p);
}
void PossionReceiver::SendToNetwork(Ptr<Packet> p){
	m_socket->SendTo(p,0,InetSocketAddress{m_peerIp,m_peerPort});
}
}
