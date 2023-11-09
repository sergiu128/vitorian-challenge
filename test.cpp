#include <cassert>
#include <iostream>

#include "msg_header.hpp"
#include "msg_login_request.hpp"

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

void TestLoginRequest() {
  // Ensure encoding/decoding works.
  {
    char buf[128];
    LoginRequest req{buf, 128, 0};

    assert((void*)req.Buffer() == (void*)buf);
    assert(req.BufferLength() == 128);
    assert(req.BufferOffset() == 0);
    assert(req.EncodedLength() == 96);

    assert(req.UserEncodingLength() == 64);
    assert(req.UserEncodingOffset() == 0);

    assert(req.PasswordEncodingLength() == 32);
    assert(req.PasswordEncodingOffset() == 64);

    assert(req.MsgType() == 'L');

    // Encode and check decoding
    const char* user = "sergiu4096@gmail.com";
    size_t user_length = strlen(user);

    const char* password = "pwd123";
    size_t password_length = strlen(password);

    req.User([&](char* slice, size_t max_length) {
         assert(max_length == LoginRequest::UserEncodingLength() - 1);
         assert((void*)slice == (void*)((char*)buf + req.UserEncodingOffset()));

         memcpy(slice, user, user_length);
         return user_length;
       })
        .Password([&](char* slice, size_t len) {
          assert(len == req.PasswordEncodingLength() - 1);
          assert((void*)slice ==
                 (void*)((char*)buf + req.PasswordEncodingOffset()));

          memcpy(slice, password, password_length);
          return password_length;
        });
    assert(strcmp(req.User(), user) == 0);
    assert(strcmp(req.Password(), password) == 0);
  }

  // Ensure the message can fit in the passed buffer.
  {
    constexpr size_t InvalidLength = LoginRequest::EncodedLength() - 1;
    char buf[InvalidLength];
    bool ok{false};
    try {
      LoginRequest req{buf, InvalidLength, 0};
    } catch (const std::exception& e) {
      ok = true;
    }
    assert(ok);
  }

  // Ensure user is truncated if it exceeds its max length.
  {
    constexpr size_t Length = LoginRequest::EncodedLength() * 2;
    char buf[Length];
    memset(buf, 0, Length);
    LoginRequest req{buf, Length, 0};

    req.User([&](char* slice, size_t max_length) {
      for (size_t i = 0; i < max_length + 2; i++) slice[i] = 'a';
      return max_length + 2;
    });
    for (size_t i = LoginRequest::UserEncodingOffset();
         i < LoginRequest::UserEncodingLength() - 1; i++) {
      assert(buf[i] == 'a');
    }
    assert(buf[LoginRequest::UserEncodingLength() - 1] == '\0');
  }

  // Ensure user is truncated if it exceeds its max length.
  {
    constexpr size_t Length = LoginRequest::EncodedLength() * 2;
    char buf[Length];
    memset(buf, 0, Length);
    LoginRequest req{buf, Length, 0};

    req.Password([&](char* slice, size_t max_length) {
      for (size_t i = 0; i < max_length + 2; i++) slice[i] = 'b';
      return max_length + 2;
    });
    for (size_t i = LoginRequest::PasswordEncodingOffset();
         i < LoginRequest::PasswordEncodingLength() - 1; i++) {
      assert(buf[i] == 'b');
    }
    assert(buf[LoginRequest::PasswordEncodingLength() - 1] == '\0');
  }

  std::cout << "TestLoginRequest done." << std::endl;
}

int main() {
  TestMsgHeader();
  TestLoginRequest();
  std::cout << "Bye." << std::endl;
}
