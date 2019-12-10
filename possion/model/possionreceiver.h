#ifndef MODEL_POSSIONRECEIVER_H_
#define MODEL_POSSIONRECEIVER_H_
#include "ns3/simulator.h"
#include "ns3/application.h"
#include "ns3/socket.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/callback.h"
namespace ns3{
class PossionReceiver:public Application{
public:
	PossionReceiver(){}
	~PossionReceiver(){}
	void Bind(uint16_t port);
	InetSocketAddress GetLocalAddress();
	typedef Callback<void,uint32_t,uint32_t> TraceOwd;
	void SetOwdTraceFuc(TraceOwd cb){
		m_traceOwdCb=cb;
	}
private:
	virtual void StartApplication() override;
	virtual void StopApplication() override;
	void RecvPacket(Ptr<Socket> socket);
	void CreateAck(uint32_t seq,uint32_t send_ts);
	void SendToNetwork(Ptr<Packet> p);
	bool m_knowPeer{false};
	bool m_running{true};
    Ipv4Address m_peerIp;
    uint16_t m_peerPort;
    uint16_t m_bindPort;
    Ptr<Socket> m_socket;
    TraceOwd m_traceOwdCb;
    uint32_t m_baseSeq{0};
    uint32_t m_lossCounter{0};
};
}
#endif /* MODEL_POSSIONRECEIVER_H_ */
