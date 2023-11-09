#include <cassert>
#include <iostream>

#include "msg_header.hpp"

void TestMsgHeader() {
  // Ensure encoding/decoding works.
  {
    char buf[128];
    MsgHeader header{buf, 128, 0};

    assert((void*)header.Buffer() == (void*)buf);
    assert(header.BufferLength() == 128);
    assert(header.BufferOffset() == 0);
    assert(header.EncodedLength() == 13);

    assert(header.MsgTypeEncodingLength() == 1);
    assert(header.MsgTypeEncodingOffset() == 0);

    assert(header.MsgLenEncodingLength() == 2);
    assert(header.MsgLenEncodingOffset() == 1);

    assert(header.TimestampEncodingLength() == 8);
    assert(header.TimestampEncodingOffset() == 3);

    assert(header.ChecksumEncodingLength() == 2);
    assert(header.ChecksumEncodingOffset() == 11);

    // Encode and check decoding
    header.MsgType('L').MsgLen(8).Timestamp(123).Checksum(64);
    assert(header.MsgType() == 'L');
    assert(header.MsgLen() == 8);
    assert(header.Timestamp() == 123);
    assert(header.Checksum() == 64);
  }

  // Ensure the message can fit in the passed buffer.
  {
    constexpr size_t InvalidLength = MsgHeader::EncodedLength() - 1;
    char buf[InvalidLength];
    bool ok{false};
    try {
      MsgHeader header{buf, InvalidLength, 0};
    } catch (const std::exception& e) {
      ok = true;
    }
    assert(ok);
  }

  std::cout << "TestMsgHeader done." << std::endl;
}

int main() {
  TestMsgHeader();
  std::cout << "Bye." << std::endl;
}
