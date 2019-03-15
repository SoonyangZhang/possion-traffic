#include "ns3/possionreceiver.h"
#include "ns3/log.h"
#include <memory.h>
#include "byte_order.h"
namespace ns3{
#define MAX_BUF_SIZE 1500
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
	NS_LOG_INFO("max "<<std::to_string(m_baseSeq)<<" loss "<<std::to_string(m_lossCounter));
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
	uint32_t recv=packet->GetSize ();
	uint8_t buf[MAX_BUF_SIZE];
	memset(buf,0,MAX_BUF_SIZE);
	packet->CopyData(buf,recv);
	uint8_t *read_ptr=buf;
	uint32_t seq=0;
	int64_t send_ts=0;
	int64_t now=Simulator::Now().GetMicroSeconds();
	memcpy((void*)&seq,(void*)read_ptr,sizeof(int32_t));
	read_ptr+=sizeof(int32_t);
	memcpy((void*)&send_ts,(void*)read_ptr,sizeof(int64_t));
	read_ptr+=sizeof(int64_t);
	seq=basic::NetToHost32(seq);
	send_ts=basic::NetToHost64(send_ts);
	//NS_LOG_INFO("recv "<<std::to_string(seq)<<" "<<std::to_string(recv));
	CreateAck(seq,send_ts);
}
void PossionReceiver::CreateAck(uint32_t seq,int64_t ts){
	uint64_t now=Simulator::Now().GetMicroSeconds();
	uint32_t owd=now-ts;//micro second;
	if(!m_traceOwdCb.IsNull()){
		m_traceOwdCb(seq,owd);
	}
	if(m_baseSeq+1<seq){
		uint32_t start=m_baseSeq+1;
		uint32_t i=0;
		m_lossCounter+=(seq-start);
	}
	if(m_baseSeq<=seq){
		m_baseSeq=seq;
	}
	uint32_t total_size=sizeof(uint32_t)+sizeof(int64_t);
	uint8_t buf[MAX_BUF_SIZE];
	uint8_t *write_ptr=buf;
	seq=basic::HostToNet32(seq);
	ts=basic::HostToNet64(ts);
	memcpy((void*)write_ptr,(void*)&seq,sizeof(uint32_t));
	write_ptr+=sizeof(uint32_t);
	memcpy((void*)write_ptr,(void*)&ts,sizeof(int64_t));
	write_ptr+=sizeof(int64_t);
	Ptr<Packet> p=Create<Packet>((uint8_t*)buf,total_size);
	SendToNetwork(p);
}
void PossionReceiver::SendToNetwork(Ptr<Packet> p){
	m_socket->SendTo(p,0,InetSocketAddress{m_peerIp,m_peerPort});
}
}
