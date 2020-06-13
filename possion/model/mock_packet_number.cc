#include "mock_packet_number.h"
namespace ns3{
MockPacketNumber::MockPacketNumber()
    : packet_number_(UninitializedPacketNumber()) {}

MockPacketNumber::MockPacketNumber(uint64_t packet_number)
    : packet_number_(packet_number) {
}

void MockPacketNumber::Clear() {
  packet_number_ = UninitializedPacketNumber();
}

void MockPacketNumber::UpdateMax(MockPacketNumber new_value) {
  if (!new_value.IsInitialized()) {
    return;
  }
  if (!IsInitialized()) {
    packet_number_ = new_value.ToUint64();
  } else {
    packet_number_ = std::max(packet_number_, new_value.ToUint64());
  }
}

uint64_t MockPacketNumber::Hash() const {
  //DCHECK(IsInitialized());
  return packet_number_;
}

uint64_t MockPacketNumber::ToUint64() const {
  //DCHECK(IsInitialized());
  return packet_number_;
}

bool MockPacketNumber::IsInitialized() const {
  return packet_number_ != UninitializedPacketNumber();
}

MockPacketNumber& MockPacketNumber::operator++() {
  packet_number_++;
  return *this;
}

MockPacketNumber MockPacketNumber::operator++(int) {
  MockPacketNumber previous(*this);
  packet_number_++;
  return previous;
}

MockPacketNumber& MockPacketNumber::operator--() {
  packet_number_--;
  return *this;
}

MockPacketNumber MockPacketNumber::operator--(int) {
  MockPacketNumber previous(*this);
  packet_number_--;
  return previous;
}

MockPacketNumber& MockPacketNumber::operator+=(uint64_t delta) {
  packet_number_ += delta;
  return *this;
}

MockPacketNumber& MockPacketNumber::operator-=(uint64_t delta) {
  packet_number_ -= delta;
  return *this;
}

std::ostream& operator<<(std::ostream& os, const MockPacketNumber& p) {
  if (p.IsInitialized()) {
    os << p.packet_number_;
  } else {
    os << "uninitialized";
  }
  return os;
}

// static
uint64_t MockPacketNumber::UninitializedPacketNumber() {
  return std::numeric_limits<uint64_t>::max();
}
}
