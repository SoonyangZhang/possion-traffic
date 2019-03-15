#include "ns3/possiontrace.h"
#include <unistd.h>
#include <memory.h>
#include "ns3/simulator.h"
namespace ns3{
PossionTrace::~PossionTrace(){
	Close();
}
void PossionTrace::OpenTraceOwdFile(std::string name){
	char buf[FILENAME_MAX];
	memset(buf,0,FILENAME_MAX);
	std::string path = std::string (getcwd(buf, FILENAME_MAX)) + "/traces/"
			+name+"_owd.txt";
	m_owd.open(path.c_str(), std::fstream::out);
}
void PossionTrace::CloseTraceOwdFile(){
	if(m_owd.is_open()){
		m_owd.close();
	}
}
void PossionTrace::OnOwd(uint32_t seq,uint32_t owd){
	char line [256];
	memset(line,0,256);
	if(m_owd.is_open()){
		float now=Simulator::Now().GetSeconds();
		sprintf (line, "%f %16d %16d",
				now,seq,owd);
		m_owd<<line<<std::endl;
	}
}
void PossionTrace::OpenTraceRttFile(std::string name){
	char buf[FILENAME_MAX];
	memset(buf,0,FILENAME_MAX);
	std::string path = std::string (getcwd(buf, FILENAME_MAX)) + "/traces/"
			+name+"_rtt.txt";
	m_rtt.open(path.c_str(), std::fstream::out);
}
void PossionTrace::CloseTraceRttFile(){
	if(m_rtt.is_open()){
		m_rtt.close();
	}
}
void PossionTrace::OnRtt(uint32_t seq,uint32_t rtt){
	char line [256];
	memset(line,0,256);
	if(m_rtt.is_open()){
		float now=Simulator::Now().GetSeconds();
		sprintf (line, "%f %16d %16d",
				now,seq,rtt);
		m_rtt<<line<<std::endl;
	}
}
void PossionTrace::OpenTraceSendGapFile(std::string name){
	char buf[FILENAME_MAX];
	memset(buf,0,FILENAME_MAX);
	std::string path = std::string (getcwd(buf, FILENAME_MAX)) + "/traces/"
			+name+"_gap.txt";
	m_gap.open(path.c_str(), std::fstream::out);
}
void PossionTrace::CloseTraceSendGapFile(){
	if(m_gap.is_open()){
		m_gap.close();
	}
}
void PossionTrace::OnGap(uint32_t gap){
	char line [256];
	memset(line,0,256);
	if(m_gap.is_open()){
		sprintf (line, "%d",gap);
		m_gap<<line<<std::endl;
	}
}
void PossionTrace::Close(){
	CloseTraceOwdFile();
	CloseTraceRttFile();
	CloseTraceSendGapFile();
}
}
