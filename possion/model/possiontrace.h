#ifndef MODEL_POSSIONTRACE_H_
#define MODEL_POSSIONTRACE_H_
#include <iostream>
#include <fstream>
#include <string>
namespace ns3{
enum PossionTraceEnable:uint8_t{
    E_POSSION_OWD=0x01,
    E_POSSION_RTT=0x02,
    E_POSSION_GAP=0x04,
    E_POSSION_ALL=E_POSSION_OWD|E_POSSION_RTT|E_POSSION_GAP,
};
class PossionTrace{
public:
	PossionTrace(){};
	~PossionTrace();
    void Log(std::string &s,uint8_t enable);
	void OnOwd(uint32_t seq,uint32_t owd);
	void OnRtt(uint32_t seq,uint32_t rtt);
	void OnGap(uint32_t gap);
private:
	void OpenTraceOwdFile(std::string &s);
    void OpenTraceRttFile(std::string &s);
    void OpenTraceSendGapFile(std::string &s);
    
    void CloseTraceOwdFile();
    void CloseTraceRttFile();
    void CloseTraceSendGapFile();
    
	void Close();
	std::fstream m_owd;
	std::fstream m_rtt;
	std::fstream m_gap;
};
}
#endif /* MODEL_POSSIONTRACE_H_ */
