#pragma once
#include <stdint.h>
#include <string>
#include "ns3/header.h"
#include "ns3/buffer.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/mock_packet_number.h"
namespace ns3{
enum MockSequenceNumberLength:uint8_t{
    MOCK_SEQ_1BYTE=1,
    MOCK_SEQ_2BYTE=2,
    MOCK_SEQ_4BYTE=4,
    MOCK_SEQ_8BYTE=8,
};
enum MockSequenceNumberLengthFlags{
  MOCK_FLAGS_1BYTE_PACKET = 0,           // 00
  MOCK_FLAGS_2BYTE_PACKET = 1,           // 01
  MOCK_FLAGS_4BYTE_PACKET = 1 << 1,      // 10
  MOCK_FLAGS_8BYTE_PACKET = 1 << 1 | 1,  // 11
};
uint8_t GetSequenceNumberLength(MockPacketNumber sequence);
uint8_t GetSequenceNumberFlags(MockPacketNumber sequence);
class MockHeader:public Header{
public:
    enum FrameType:uint8_t{
        FRAMEMIN,
        STREAM,
        ACK,
        FRAMEMAX,
    };
    MockHeader(){}
    MockHeader(MockPacketNumber sequence,uint32_t event_time,uint8_t type);
    ~MockHeader(){}

    static TypeId GetTypeId (void);
    virtual TypeId GetInstanceTypeId (void) const;
    virtual void Print (std::ostream &os) const;
    virtual uint32_t GetSerializedSize (void) const;
    virtual void Serialize (Buffer::Iterator start) const;
    virtual uint32_t Deserialize (Buffer::Iterator start);

    FrameType GetFrameType(){return type_;}
    MockPacketNumber GetFrameSequence(){return sequence_;}
    uint32_t GetTimeStamp() {return event_time_;}
private:
    std::string TypeToString() const;
    void WriteSequenceNumber(Buffer::Iterator& i, const void *value,uint32_t size) const;
    void ReadSequenceNumber(Buffer::Iterator& i, void *value,uint32_t size);
    FrameType type_{FRAMEMIN};
    MockPacketNumber sequence_;
    uint32_t event_time_{0};
};
}

