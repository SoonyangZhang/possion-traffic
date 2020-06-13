#pragma once
#include <limits>
#include <cstdint>
#include <ostream>
namespace ns3{
class  MockPacketNumber {
 public:
  // Construct an uninitialized packet number.
  MockPacketNumber();
  // Construct a packet number from uint64_t. |packet_number| cannot equal the
  // sentinel value.
  explicit MockPacketNumber(uint64_t packet_number);

  // Packet number becomes uninitialized after calling this function.
  void Clear();

  // Updates this packet number to be |new_value| if it is greater than current
  // value.
  void UpdateMax(MockPacketNumber new_value);

  // REQUIRES: IsInitialized() == true.
  uint64_t Hash() const;

  // Converts packet number to uint64_t.
  // REQUIRES: IsInitialized() == true.
  uint64_t ToUint64() const;

  // Returns true if packet number is considered initialized.
  bool IsInitialized() const;

  // REQUIRES: IsInitialized() == true && ToUint64() <
  // numeric_limits<uint64_t>::max() - 1.
  MockPacketNumber& operator++();
  MockPacketNumber operator++(int);
  // REQUIRES: IsInitialized() == true && ToUint64() >= 1.
  MockPacketNumber& operator--();
  MockPacketNumber operator--(int);

  // REQUIRES: IsInitialized() == true && numeric_limits<uint64_t>::max() -
  // ToUint64() > |delta|.
  MockPacketNumber& operator+=(uint64_t delta);
  // REQUIRES: IsInitialized() == true && ToUint64() >= |delta|.
  MockPacketNumber& operator-=(uint64_t delta);

  friend std::ostream& operator<<(
      std::ostream& os,
      const MockPacketNumber& p);

 private:
  // All following operators REQUIRE operands.Initialized() == true.
  friend inline bool operator==(MockPacketNumber lhs, MockPacketNumber rhs);
  friend inline bool operator!=(MockPacketNumber lhs, MockPacketNumber rhs);
  friend inline bool operator<(MockPacketNumber lhs, MockPacketNumber rhs);
  friend inline bool operator<=(MockPacketNumber lhs, MockPacketNumber rhs);
  friend inline bool operator>(MockPacketNumber lhs, MockPacketNumber rhs);
  friend inline bool operator>=(MockPacketNumber lhs, MockPacketNumber rhs);

  // REQUIRES: numeric_limits<uint64_t>::max() - lhs.ToUint64() > |delta|.
  friend inline MockPacketNumber operator+(MockPacketNumber lhs,
                                           uint64_t delta);
  // REQUIRES: lhs.ToUint64() >= |delta|.
  friend inline MockPacketNumber operator-(MockPacketNumber lhs,
                                           uint64_t delta);
  // REQUIRES: lhs >= rhs.
  friend inline uint64_t operator-(MockPacketNumber lhs, MockPacketNumber rhs);

  // The sentinel value representing an uninitialized packet number.
  static uint64_t UninitializedPacketNumber();

  uint64_t packet_number_;
};

class QuicPacketNumberHash {
 public:
  uint64_t operator()(MockPacketNumber packet_number) const noexcept {
    return packet_number.Hash();
  }
};

inline bool operator==(MockPacketNumber lhs, MockPacketNumber rhs) {
  //DCHECK(lhs.IsInitialized() && rhs.IsInitialized()) << lhs << " vs. " << rhs;
  return lhs.packet_number_ == rhs.packet_number_;
}

inline bool operator!=(MockPacketNumber lhs, MockPacketNumber rhs) {
 // DCHECK(lhs.IsInitialized() && rhs.IsInitialized()) << lhs << " vs. " << rhs;
  return lhs.packet_number_ != rhs.packet_number_;
}

inline bool operator<(MockPacketNumber lhs, MockPacketNumber rhs) {
  //DCHECK(lhs.IsInitialized() && rhs.IsInitialized()) << lhs << " vs. " << rhs;
  return lhs.packet_number_ < rhs.packet_number_;
}

inline bool operator<=(MockPacketNumber lhs, MockPacketNumber rhs) {
  //DCHECK(lhs.IsInitialized() && rhs.IsInitialized()) << lhs << " vs. " << rhs;
  return lhs.packet_number_ <= rhs.packet_number_;
}

inline bool operator>(MockPacketNumber lhs, MockPacketNumber rhs) {
  //DCHECK(lhs.IsInitialized() && rhs.IsInitialized()) << lhs << " vs. " << rhs;
  return lhs.packet_number_ > rhs.packet_number_;
}

inline bool operator>=(MockPacketNumber lhs, MockPacketNumber rhs) {
  //DCHECK(lhs.IsInitialized() && rhs.IsInitialized()) << lhs << " vs. " << rhs;
  return lhs.packet_number_ >= rhs.packet_number_;
}

inline MockPacketNumber operator+(MockPacketNumber lhs, uint64_t delta) {
  return MockPacketNumber(lhs.packet_number_ + delta);
}

inline MockPacketNumber operator-(MockPacketNumber lhs, uint64_t delta) {
  return MockPacketNumber(lhs.packet_number_ - delta);
}

inline uint64_t operator-(MockPacketNumber lhs, MockPacketNumber rhs) {
 // DCHECK(lhs.IsInitialized() && rhs.IsInitialized() && lhs >= rhs)
 //     << lhs << " vs. " << rhs;
  return lhs.packet_number_ - rhs.packet_number_;
}
}
