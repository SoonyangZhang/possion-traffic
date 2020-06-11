#include <memory.h>
#include <string>
#include "ns3/possionsender.h"
#include "ns3/log.h"
#include "byte_order.h"
#include "mock_proto.h"
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
namespace ns3{
NS_LOG_COMPONENT_DEFINE("PossionSender");
PossionSender::PossionSender(uint32_t bps):PossionSender(bps,1000){
	//PossionSender(bps,1000);// this introduces bug;
}
PossionSender::PossionSender(uint32_t bps,uint32_t mtu){
    m_seq=MockPacketNumber(1);
	m_bps=bps;
	m_packetSize=mtu;
	m_interval=((double)m_packetSize*8*1000)/(m_bps);
	m_lambda=1.0/m_interval;
	set_seed();
}
PossionSender::~PossionSender(){
	m_trans_infos.clear();
}
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
		CreatePacket(m_packetSize);
		double micro=e_random(m_lambda)*1000;
		Time next=MicroSeconds(micro);
		m_packetsTimer=Simulator::Schedule(next,
				&PossionSender::TimerCallback,this);
	}
}
void PossionSender::CreatePacket(uint32_t size){
    Ptr<Packet> p=Create<Packet>(size);
	uint32_t event_time=Simulator::Now().GetMilliSeconds();
    MockHeader header(m_seq,event_time,MockHeader::STREAM);
	NS_ASSERT(size>header.GetSerializedSize());
    p->AddHeader(header);
	SendToNetwork(p);
    if(!m_least_unacked.IsInitialized()){
        m_least_unacked=m_seq;
    }
    m_trans_infos.emplace_back(m_seq.ToUint64(),event_time,size);
	m_seq++;
}
void PossionSender::RecvPacket(Ptr<Socket> socket){
	if(!m_running){
		return;
	}
    uint32_t now=Simulator::Now().GetMilliSeconds();
	Address remoteAddr;
	auto packet = socket->RecvFrom (remoteAddr);
    MockHeader header;
    packet->RemoveHeader(header);
    if(header.GetFrameType()==MockHeader::ACK){
        MockPacketNumber seq=header.GetFrameSequence();
        uint32_t receive_ts=header.GetTimeStamp();
        if(seq>=m_least_unacked){
            TransmissionInfo *info=GetTransmissionInfo(seq);
            if(info){
                uint32_t sent_time=info->sent_time;
                uint32_t rtt=(now-sent_time);
                uint32_t owd=receive_ts-sent_time;
                if(!m_traceRttCb.IsNull()){
                    m_traceRttCb((uint32_t)seq.ToUint64(),rtt);
                }
                if(!m_traceSendOwdCb.IsNull()){
                    m_traceSendOwdCb((uint32_t)seq.ToUint64(),owd);
                }
                uint32_t acked=seq-m_least_unacked;
                uint32_t i=0;
                for(i=0;i<=acked;i++){
                    m_trans_infos.pop_front();
                    m_least_unacked++;
                }                
            }

        }
    }
}
void PossionSender::SendToNetwork(Ptr<Packet> p){
	m_socket->SendTo(p,0,InetSocketAddress{m_peerIp,m_peerPort});
}
PossionSender::TransmissionInfo *PossionSender::GetTransmissionInfo(MockPacketNumber seq){
    TransmissionInfo *info=nullptr;
    if(!m_least_unacked.IsInitialized()||seq<m_least_unacked||(m_least_unacked+m_trans_infos.size())<=seq){
        //NS_LOG_INFO("null info unack "<<least_unacked_<<" "<<seq);
        return info;
    }
    info=&m_trans_infos[seq-m_least_unacked];
    return info;
}
}



