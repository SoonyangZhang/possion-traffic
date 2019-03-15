#include "ns3/possionsender.h"
#include "ns3/log.h"
#include <memory.h>
#include <string>

#include "byte_order.h"

#include <math.h>
#include <time.h>
#include <stdlib.h> //RAND_MAX
//exponential distribution;
static uint8_t seed_set_flag=0;
void set_seed(){
	if(!seed_set_flag){
	srand((unsigned)time(NULL));
	}
	seed_set_flag=1;
}
double e_random(double lambda){
    double ret=0.0;
    double u=0.0;
    do{
        u=(double)rand()/(double)RAND_MAX;;
    }while(u<=0||u>1);
    ret=(-1.0/lambda)*log(u);
    return ret;
}
#define MAX_BUF_SIZE 1500
namespace ns3{
NS_LOG_COMPONENT_DEFINE("PossionSender");
PossionSender::PossionSender(uint32_t bps):PossionSender(bps,1000){
	//PossionSender(bps,1000);// this introduces bug;
}
PossionSender::PossionSender(uint32_t bps,uint32_t mtu){
	m_bps=bps;
	m_packetSize=mtu;
	m_interval=((double)m_packetSize*8*1000)/(m_bps);
	m_lambda=1.0/m_interval;
	set_seed();
}
PossionSender::~PossionSender(){}
void PossionSender::Bind(uint16_t port){
    if (m_socket== NULL) {
        m_socket = Socket::CreateSocket (GetNode (),UdpSocketFactory::GetTypeId ());
        auto local = InetSocketAddress{Ipv4Address::GetAny (), port};
        auto res = m_socket->Bind (local);
        NS_ASSERT (res == 0);
    }
    m_bindPort=port;
    m_socket->SetRecvCallback (MakeCallback(&PossionSender::RecvPacket,this));
}
InetSocketAddress PossionSender::GetLocalAddress(){
    Ptr<Node> node=GetNode();
    Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
    Ipv4Address local_ip = ipv4->GetAddress (1, 0).GetLocal ();
	return InetSocketAddress{local_ip,m_bindPort};
}
void PossionSender::ConfigurePeer(Ipv4Address addr,uint16_t port){
	m_peerIp=addr;
	m_peerPort=port;
}
void PossionSender::StartApplication(){
	m_packetsTimer=Simulator::ScheduleNow(&PossionSender::TimerCallback,this);
}
void PossionSender::StopApplication(){
	m_running=false;
}
void PossionSender::TimerCallback(){
	if(!m_running){
		return;
	}
	if(m_packetsTimer.IsExpired()){
		int64_t now=Simulator::Now().GetMicroSeconds();
		if(m_lastSendTs!=0){
			uint32_t gap=now-m_lastSendTs;
			if(!m_traceGapCb.IsNull()){
				m_traceGapCb(gap);
			}
		}
		m_lastSendTs=now;
		CreatePacket(m_packetSize,now);
		double micro=e_random(m_lambda)*1000;
		Time next=MicroSeconds(micro);
		m_packetsTimer=Simulator::Schedule(next,
				&PossionSender::TimerCallback,this);
	}
}
void PossionSender::CreatePacket(uint32_t size,int64_t now){
	uint8_t buf[MAX_BUF_SIZE];
	memset(buf,0,MAX_BUF_SIZE);
	uint8_t *write_ptr=buf;
	uint32_t seq=basic::HostToNet32(m_seq);
	memcpy((void*)write_ptr,(void*)&seq,sizeof(uint32_t));
	write_ptr+=sizeof(uint32_t);
	uint64_t ts=basic::HostToNet64(now);
	memcpy((void*)write_ptr,(void*)&ts,sizeof(int64_t));
	write_ptr+=sizeof(int64_t);
	Ptr<Packet> p=Create<Packet>((uint8_t*)buf,m_packetSize);
	SendToNetwork(p);
	m_seq++;
}
void PossionSender::RecvPacket(Ptr<Socket> socket){
	if(!m_running){
		return;
	}
	Address remoteAddr;
	auto packet = socket->RecvFrom (remoteAddr);
	uint32_t recv=packet->GetSize ();
	uint8_t *buf=new uint8_t[recv];
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
	uint32_t gap=(now-send_ts);
	delete buf;
	if(!m_traceRttCb.IsNull()){
		m_traceRttCb(seq,gap);
	}
}
void PossionSender::SendToNetwork(Ptr<Packet> p){
	m_socket->SendTo(p,0,InetSocketAddress{m_peerIp,m_peerPort});
}
}



