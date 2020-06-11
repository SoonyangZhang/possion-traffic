#include "mock_proto.h"
#include "ns3/log.h"
#include "byte_order.h"
namespace ns3{
const uint8_t kSequenceNumberShift=4;
NS_LOG_COMPONENT_DEFINE ("MockHeader");
NS_OBJECT_ENSURE_REGISTERED (MockHeader);
MockSequenceNumberLength ReadSequenceNumberLength(uint8_t flags){
    switch (flags & MOCK_FLAGS_8BYTE_PACKET) {
    case MOCK_FLAGS_8BYTE_PACKET:
        return MOCK_SEQ_8BYTE;
    case MOCK_FLAGS_4BYTE_PACKET:
        return MOCK_SEQ_4BYTE;
    case MOCK_FLAGS_2BYTE_PACKET:
        return MOCK_SEQ_2BYTE;
    case MOCK_FLAGS_1BYTE_PACKET:
        return MOCK_SEQ_1BYTE;
    default:
        return MOCK_SEQ_8BYTE;
  }
}
uint8_t GetSequenceNumberLength(MockPacketNumber sequence){
    if(sequence<MockPacketNumber(UINT32_C(1)<<(MOCK_SEQ_1BYTE*8))){
        return MOCK_SEQ_1BYTE;
    }else if(sequence<MockPacketNumber(UINT32_C(1)<<(MOCK_SEQ_2BYTE*8))){
        return MOCK_SEQ_2BYTE;
    }else if(sequence<MockPacketNumber(UINT64_C(1)<<(MOCK_SEQ_4BYTE*8))){
        return MOCK_SEQ_4BYTE;
    }else if(sequence<MockPacketNumber()){
        return MOCK_SEQ_8BYTE;
    }
    return 0;
}
uint8_t GetSequenceNumberFlags(MockPacketNumber sequence){
    uint8_t seq_length=GetSequenceNumberLength(sequence);
    NS_ASSERT(seq_length);
    switch(seq_length){
        case MOCK_SEQ_1BYTE:
        return MOCK_FLAGS_1BYTE_PACKET;
        case MOCK_SEQ_2BYTE:
        return MOCK_FLAGS_2BYTE_PACKET;
        case MOCK_SEQ_4BYTE:
        return MOCK_FLAGS_4BYTE_PACKET;
        case MOCK_SEQ_8BYTE:
        return MOCK_FLAGS_8BYTE_PACKET;
    }
    
}
MockHeader::MockHeader(MockPacketNumber sequence,uint32_t event_time,uint8_t type){
    type_=(FrameType)type;
    sequence_=sequence;
    event_time_=event_time;
}
TypeId MockHeader::GetTypeId ()
{
    static TypeId tid = TypeId ("MockHeader")
      .SetParent<Header> ()
      .AddConstructor<MockHeader> ()
    ;
    return tid;
}
TypeId MockHeader::GetInstanceTypeId (void) const
{
	return GetTypeId();
}
void MockHeader::Print (std::ostream &os) const{
    os<<TypeToString();
}
uint32_t MockHeader::GetSerializedSize () const{
    uint32_t len=1;
    uint32_t seq_length=GetSequenceNumberLength(sequence_);
    len+=(seq_length+sizeof(event_time_));
    NS_ASSERT(seq_length);
    return len;
}
void MockHeader::Serialize (Buffer::Iterator start) const{
    uint8_t public_flags=0;
    Buffer::Iterator i = start;
    public_flags=(uint8_t)(type_<<kSequenceNumberShift);
    public_flags|=GetSequenceNumberFlags(sequence_);
    i.WriteU8(public_flags);
    uint32_t seq_length=GetSequenceNumberLength(sequence_);
	uint64_t number=sequence_.ToUint64();
    number=basic::HostToNet64(number);
    WriteSequenceNumber(i,reinterpret_cast<char*>(&number)+sizeof(number)-seq_length,seq_length);
	i.WriteU32(event_time_);
}
uint32_t MockHeader::Deserialize (Buffer::Iterator start){
    uint8_t public_flags=0;
    Buffer::Iterator i = start;
    public_flags=i.ReadU8();
	uint64_t number=0;
    uint32_t seq_length=ReadSequenceNumberLength(public_flags);
    type_=(FrameType)((public_flags&0xF0)>>kSequenceNumberShift);
    ReadSequenceNumber(i,reinterpret_cast<char*>(&number)+sizeof(number)-seq_length,seq_length);
    number=basic::NetToHost64(number);
    //uint64_t number=i.ReadU32();
    event_time_=i.ReadU32();
	sequence_=MockPacketNumber(number);
    return GetSerializedSize();
}
std::string MockHeader::TypeToString() const{
    static const char* TypeNames[4]={
        "frame min",
        "frame stream",
        "frame ack",
        "frame max"
    };
    return std::string(TypeNames[type_]);
}
void MockHeader::WriteSequenceNumber(Buffer::Iterator& i, const void *value,uint32_t size) const{
    NS_ASSERT(size);
    uint8_t buf[8];
    memcpy(buf,value,size);
    for(uint32_t index=0;index<size;index++){
        i.WriteU8(buf[index]);
    }
}
void MockHeader::ReadSequenceNumber(Buffer::Iterator& i, void *value,uint32_t size){
    uint8_t buf[8];
	NS_LOG_INFO("r "<<size);
    for(uint32_t index=0;index<size;index++){
        buf[index]=i.ReadU8();
    }
    memcpy(value,buf,size);
}
}

