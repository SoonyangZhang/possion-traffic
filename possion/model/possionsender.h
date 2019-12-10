#ifndef MODEL_POSSIONSENDER_H_
#define MODEL_POSSIONSENDER_H_
#include "ns3/event-id.h"
#include "ns3/callback.h"
#include "ns3/application.h"
#include "ns3/socket.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
namespace ns3{
class PossionSender :public Application{
public:
	PossionSender(uint32_t bps);
	PossionSender(uint32_t bps,uint32_t mtu);
	~PossionSender();
	void Bind(uint16_t port);
	InetSocketAddress GetLocalAddress();
	void ConfigurePeer(Ipv4Address addr,uint16_t port);
	typedef Callback<void,uint32_t,uint32_t> TraceRtt;
	void SetTraceRttFun(TraceRtt cb){
		m_traceRttCb=cb;
	}
	typedef Callback<void,uint32_t,uint32_t> TraceSendOwd;
	void SetTraceSendOwdFun(TraceSendOwd cb){
		m_traceSendOwdCb=cb;
	}
	typedef Callback<void,uint32_t> TraceGap;
	void SetTraceGapFun(TraceGap cb){
		m_traceGapCb=cb;
	}
private:
	virtual void StartApplication() override;
	virtual void StopApplication() override;
	void TimerCallback();
	void CreatePacket(uint32_t size);
	void RecvPacket(Ptr<Socket> socket);
	void SendToNetwork(Ptr<Packet> p);
	bool m_running{true};
	uint32_t m_seq{0};
	uint32_t m_bps;
	uint32_t m_packetSize{0};
	double m_interval; //in unit of millisecond;
	double m_lambda;
	EventId m_packetsTimer;
    Ipv4Address m_peerIp;
    uint16_t m_peerPort;
    uint16_t m_bindPort;
    Ptr<Socket> m_socket;
    TraceRtt m_traceRttCb;
    TraceSendOwd m_traceSendOwdCb;
    TraceGap m_traceGapCb;
    int64_t m_lastSendTs{0};
};
}
#endif /* MODEL_POSSIONSENDER_H_ */
