#ifndef MODEL_POSSIONTRACE_H_
#define MODEL_POSSIONTRACE_H_
#include <iostream>
#include <fstream>
#include <string>
namespace ns3{
class PossionTrace{
public:
	PossionTrace(){}
	~PossionTrace();
	void OpenTraceOwdFile(std::string name);
	void CloseTraceOwdFile();
	void OnOwd(uint32_t seq,uint32_t owd);
	void OpenTraceRttFile(std::string name);
	void CloseTraceRttFile();
	void OnRtt(uint32_t seq,uint32_t rtt);
	void OpenTraceSendGapFile(std::string name);
	void CloseTraceSendGapFile();
	void OnGap(uint32_t gap);
private:
	void Close();
	std::fstream m_owd;
	std::fstream m_rtt;
	std::fstream m_gap;
};
}
#endif /* MODEL_POSSIONTRACE_H_ */
