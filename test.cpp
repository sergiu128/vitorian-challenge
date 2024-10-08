#include <unistd.h>

#include <atomic>
#include <cassert>
#include <functional>
#include <iostream>
#include <memory>
#include <thread>

#include "checksum.hpp"
#include "msg_header.hpp"
#include "msg_login_request.hpp"
#include "msg_login_response.hpp"
#include "msg_logout_request.hpp"
#include "msg_logout_response.hpp"
#include "msg_submission_request.hpp"
#include "msg_submission_response.hpp"
#include "proto_client.hpp"
#include "resolver.hpp"
#include "tcp_client.hpp"
#include "tcp_server.hpp"
#include "util.hpp"

void TestMsgHeader() {
  // Ensure encoding/decoding works.
  {
    char buf[1024];
    memset(buf, 0, 1024);
    MsgHeader header{buf, 1024, 128};

    assert((void*)header.Buffer() == (void*)buf);
    assert(header.BufferLength() == 1024);
    assert(header.BufferOffset() == 128);
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

    for (size_t i = 0; i < 128; i++) assert(buf[i] == 0);
    for (size_t i = 128 + header.EncodedLength(); i < 1024; i++)
      assert(buf[i] == 0);
  }

  // Ensure the message can fit in the passed buffer.
  {
    constexpr size_t kInvalidLength = MsgHeader::EncodedLength() - 1;
    char buf[kInvalidLength];
    bool ok{false};
    try {
      MsgHeader header{buf, kInvalidLength, 0};
    } catch (const std::exception& e) {
      ok = true;
    }
    assert(ok);
  }

  std::cout << "TestMsgHeader done." << std::endl;
}

void TestLoginRequest() {
  assert(LoginRequest::EncodedLength() + MsgHeader::EncodedLength() == 109);

  // Ensure encoding/decoding works.
  {
    char buf[1024];
    memset(buf, 0, 1024);
    LoginRequest req{buf, 1024, 128};

    assert((void*)req.Buffer() == (void*)buf);
    assert(req.BufferLength() == 1024);
    assert(req.BufferOffset() == 128);
    assert(req.EncodedLength() == 96);

    assert(req.UserEncodingLength() == 64);
    assert(req.UserEncodingOffset() == 0);

    assert(req.PasswordEncodingLength() == 32);
    assert(req.PasswordEncodingOffset() == 64);

    assert(req.MsgType() == 'L');

    // Encode and check decoding
    req.User("sergiu4096@gmail.com").Password("pwd123");
    assert(req.User() == "sergiu4096@gmail.com");
    assert(req.Password() == "pwd123");
    assert(req.Password().size() == 6);

    for (size_t i = 0; i < 128; i++) assert(buf[i] == 0);
    for (size_t i = 128 + req.EncodedLength(); i < 1024; i++)
      assert(buf[i] == 0);
  }

  // Ensure the message can fit in the passed buffer.
  {
    constexpr size_t kInvalidLength = LoginRequest::EncodedLength() - 1;
    char buf[kInvalidLength];
    bool ok{false};
    try {
      LoginRequest req{buf, kInvalidLength, 0};
    } catch (const std::exception& e) {
      ok = true;
    }
    assert(ok);
  }

  // Ensure user bytes are all zeroes if user is empty
  {
    constexpr size_t kLength = LoginRequest::EncodedLength();
    char buf[kLength];
    memset(buf, 'a', kLength);
    LoginRequest req{buf, kLength, 0};

    req.User("");

    size_t start = LoginRequest::UserEncodingOffset();
    size_t end = start + LoginRequest::UserEncodingLength();
    for (size_t i = 0; i < start; i++) assert(buf[i] == 'a');
    for (size_t i = start; i < end - 1; i++) assert(buf[i] == 0);
    for (size_t i = end; i < kLength; i++) assert(buf[i] == 'a');
  }

  // Ensure user is truncated if it exceeds its max length.
  {
    constexpr size_t kLength = LoginRequest::EncodedLength() * 2;
    char buf[kLength];
    memset(buf, 0, kLength);
    LoginRequest req{buf, kLength, 0};

    req.User(std::string(LoginRequest::UserEncodingLength() * 2, 'a'));

    size_t start = LoginRequest::UserEncodingOffset();
    size_t end = start + LoginRequest::UserEncodingLength();
    for (size_t i = 0; i < start; i++) assert(buf[i] == 0);
    for (size_t i = start; i < end - 1; i++) assert(buf[i] == 'a');
    for (size_t i = end; i < kLength; i++) assert(buf[i] == 0);
  }

  // Ensure password bytes are all zeroes if password is empty
  {
    constexpr size_t kLength = LoginRequest::EncodedLength();
    char buf[kLength];
    memset(buf, 'a', kLength);
    LoginRequest req{buf, kLength, 0};

    req.Password("");

    size_t start = LoginRequest::PasswordEncodingOffset();
    size_t end = start + LoginRequest::PasswordEncodingLength();
    for (size_t i = 0; i < start; i++) assert(buf[i] == 'a');
    for (size_t i = start; i < end - 1; i++) assert(buf[i] == 0);
    for (size_t i = end; i < kLength; i++) assert(buf[i] == 'a');
  }

  // Ensure password is truncated if it exceeds its max length.
  {
    constexpr size_t kLength = LoginRequest::EncodedLength() * 2;
    char buf[kLength];
    memset(buf, 0, kLength);
    LoginRequest req{buf, kLength, 0};

    req.Password(std::string(LoginRequest::PasswordEncodingLength() * 2, 'a'));

    size_t start = LoginRequest::PasswordEncodingOffset();
    size_t end = start + LoginRequest::PasswordEncodingLength();
    for (size_t i = 0; i < start; i++) assert(buf[i] == 0);
    for (size_t i = start; i < end - 1; i++) assert(buf[i] == 'a');
    for (size_t i = end; i < kLength; i++) assert(buf[i] == 0);
  }

  std::cout << "TestLoginRequest done." << std::endl;
}

void TestLoginResponse() {
  assert(LoginResponse::EncodedLength() + MsgHeader::EncodedLength() == 46);

  // Ensure encoding/decoding works.
  {
    char buf[1024];
    memset(buf, 0, 1024);
    LoginResponse res{buf, 1024, 128};

    assert((void*)res.Buffer() == (void*)buf);
    assert(res.BufferLength() == 1024);
    assert(res.BufferOffset() == 128);
    assert(res.EncodedLength() == 33);

    assert(res.CodeEncodingLength() == 1);
    assert(res.CodeEncodingOffset() == 0);

    assert(res.ReasonEncodingLength() == 32);
    assert(res.ReasonEncodingOffset() == 1);

    assert(res.MsgType() == 'E');

    // Encode and check decoding
    res.Code('Y').Reason("no reason");
    assert(res.Code() == 'Y');
    assert(res.Reason() == "no reason");

    for (size_t i = 0; i < 128; i++) assert(buf[i] == 0);
    for (size_t i = 128 + res.EncodedLength(); i < 1024; i++)
      assert(buf[i] == 0);
  }

  // Ensure the message can fit in the passed buffer.
  {
    constexpr size_t kInvalidLength = LoginResponse::EncodedLength() - 1;
    char buf[kInvalidLength];
    bool ok{false};
    try {
      LoginResponse req{buf, kInvalidLength, 0};
    } catch (const std::exception& e) {
      ok = true;
    }
    assert(ok);
  }

  // Ensure reason bytes are all zeroes if reason is empty
  {
    constexpr size_t kLength = LoginResponse::EncodedLength();
    char buf[kLength];
    memset(buf, 'a', kLength);
    LoginResponse req{buf, kLength, 0};

    req.Reason("");

    size_t start = LoginResponse::ReasonEncodingOffset();
    size_t end = start + LoginResponse::ReasonEncodingLength();
    for (size_t i = 0; i < start; i++) assert(buf[i] == 'a');
    for (size_t i = start; i < end - 1; i++) assert(buf[i] == 0);
    for (size_t i = end; i < kLength; i++) assert(buf[i] == 'a');
  }

  // Ensure reason is truncated if it exceeds its max length.
  {
    constexpr size_t kLength = LoginResponse::EncodedLength() * 2;
    char buf[kLength];
    memset(buf, 0, kLength);
    LoginResponse res{buf, kLength, 0};

    res.Reason(std::string(LoginResponse::ReasonEncodingLength() * 2, 'a'));

    size_t start = LoginResponse::ReasonEncodingOffset();
    size_t end = start + LoginResponse::ReasonEncodingLength();
    for (size_t i = 0; i < start; i++) assert(buf[i] == 0);
    for (size_t i = start; i < end - 1; i++) assert(buf[i] == 'a');
    for (size_t i = end; i < kLength; i++) assert(buf[i] == 0);
  }

  std::cout << "TestLoginResponse done." << std::endl;
}

void TestSubmissionRequest() {
  assert(SubmissionRequest::EncodedLength() + MsgHeader::EncodedLength() ==
         205);

  // Ensure encoding/decoding works.
  {
    char buf[1024];
    memset(buf, 0, 1024);
    SubmissionRequest req{buf, 1024, 128};

    assert((void*)req.Buffer() == (void*)buf);
    assert(req.BufferLength() == 1024);
    assert(req.BufferOffset() == 128);
    assert(req.EncodedLength() == 3 * 64);

    assert(req.NameEncodingLength() == 64);
    assert(req.NameEncodingOffset() == 0);

    assert(req.EmailEncodingLength() == 64);
    assert(req.EmailEncodingOffset() == 64);

    assert(req.RepoEncodingLength() == 64);
    assert(req.RepoEncodingOffset() == 128);

    assert(req.MsgType() == 'S');

    // Encode and check decoding
    req.Name("Sergiu Marin")
        .Email("sergiu4096@gmail.com")
        .Repo("https://github.com/sergiu128/vitorian-challenge");
    assert(req.Name() == "Sergiu Marin");
    assert(req.Email() == "sergiu4096@gmail.com");
    assert(req.Repo() == "https://github.com/sergiu128/vitorian-challenge");

    for (size_t i = 0; i < 128; i++) assert(buf[i] == 0);
    for (size_t i = 128 + req.EncodedLength(); i < 1024; i++)
      assert(buf[i] == 0);
  }

  // Ensure the message can fit in the passed buffer.
  {
    constexpr size_t kInvalidLength = SubmissionRequest::EncodedLength() - 1;
    char buf[kInvalidLength];
    bool ok{false};
    try {
      SubmissionRequest req{buf, kInvalidLength, 0};
    } catch (const std::exception& e) {
      ok = true;
    }
    assert(ok);
  }

  // Ensure name bytes are all zeroes if name is empty
  {
    constexpr size_t kLength = SubmissionRequest::EncodedLength();
    char buf[kLength];
    memset(buf, 'a', kLength);
    SubmissionRequest req{buf, kLength, 0};

    req.Name("");

    size_t start = SubmissionRequest::NameEncodingOffset();
    size_t end = start + SubmissionRequest::NameEncodingLength();
    for (size_t i = 0; i < start; i++) assert(buf[i] == 'a');
    for (size_t i = start; i < end - 1; i++) assert(buf[i] == 0);
    for (size_t i = end; i < kLength; i++) assert(buf[i] == 'a');
  }

  // Ensure name is truncated if it exceeds its max length.
  {
    constexpr size_t kLength = SubmissionRequest::EncodedLength() * 2;
    char buf[kLength];
    memset(buf, 0, kLength);
    SubmissionRequest req{buf, kLength, 0};

    req.Name(std::string(SubmissionRequest::NameEncodingLength() * 2, 'a'));

    size_t start = SubmissionRequest::NameEncodingOffset();
    size_t end = start + SubmissionRequest::NameEncodingLength();
    for (size_t i = 0; i < start; i++) assert(buf[i] == 0);
    for (size_t i = start; i < end - 1; i++) assert(buf[i] == 'a');
    for (size_t i = end; i < kLength; i++) assert(buf[i] == 0);
  }

  // Ensure email bytes are all zeroes if email is empty
  {
    constexpr size_t kLength = SubmissionRequest::EncodedLength();
    char buf[kLength];
    memset(buf, 'a', kLength);
    SubmissionRequest req{buf, kLength, 0};

    req.Email("");

    size_t start = SubmissionRequest::EmailEncodingOffset();
    size_t end = start + SubmissionRequest::EmailEncodingLength();
    for (size_t i = 0; i < start; i++) assert(buf[i] == 'a');
    for (size_t i = start; i < end - 1; i++) assert(buf[i] == 0);
    for (size_t i = end; i < kLength; i++) assert(buf[i] == 'a');
  }

  // Ensure email is truncated if it exceeds its max length.
  {
    constexpr size_t kLength = SubmissionRequest::EncodedLength() * 2;
    char buf[kLength];
    memset(buf, 0, kLength);
    SubmissionRequest req{buf, kLength, 0};

    req.Email(std::string(SubmissionRequest::EmailEncodingLength() * 2, 'a'));

    size_t start = SubmissionRequest::EmailEncodingOffset();
    size_t end = start + SubmissionRequest::EmailEncodingLength();
    for (size_t i = 0; i < start; i++) assert(buf[i] == 0);
    for (size_t i = start; i < end - 1; i++) assert(buf[i] == 'a');
    for (size_t i = end; i < kLength; i++) assert(buf[i] == 0);
  }

  // Ensure repo bytes are all zeroes if repo is empty
  {
    constexpr size_t kLength = SubmissionRequest::EncodedLength();
    char buf[kLength];
    memset(buf, 'a', kLength);
    SubmissionRequest req{buf, kLength, 0};

    req.Repo("");

    size_t start = SubmissionRequest::RepoEncodingOffset();
    size_t end = start + SubmissionRequest::RepoEncodingLength();
    for (size_t i = 0; i < start; i++) assert(buf[i] == 'a');
    for (size_t i = start; i < end - 1; i++) assert(buf[i] == 0);
    for (size_t i = end; i < kLength; i++) assert(buf[i] == 'a');
  }

  // Ensure repo is truncated if it exceeds its max length.
  {
    constexpr size_t kLength = SubmissionRequest::EncodedLength() * 2;
    char buf[kLength];
    memset(buf, 0, kLength);
    SubmissionRequest req{buf, kLength, 0};

    req.Repo(std::string(SubmissionRequest::RepoEncodingLength() * 2, 'a'));

    size_t start = SubmissionRequest::RepoEncodingOffset();
    size_t end = start + SubmissionRequest::RepoEncodingLength();
    for (size_t i = 0; i < start; i++) assert(buf[i] == 0);
    for (size_t i = start; i < end - 1; i++) assert(buf[i] == 'a');
    for (size_t i = end; i < kLength; i++) assert(buf[i] == 0);
  }

  std::cout << "TestSubmissionRequest done." << std::endl;
}

void TestSubmissionResponse() {
  assert(SubmissionResponse::EncodedLength() + MsgHeader::EncodedLength() ==
         45);

  // Ensure encoding/decoding works.
  {
    char buf[1024];
    memset(buf, 0, 1024);
    SubmissionResponse res{buf, 1024, 128};

    assert((void*)res.Buffer() == (void*)buf);
    assert(res.BufferLength() == 1024);
    assert(res.BufferOffset() == 128);
    assert(res.EncodedLength() == 32);

    assert(res.TokenEncodingLength() == 32);
    assert(res.TokenEncodingOffset() == 0);

    assert(res.MsgType() == 'R');

    // Encode and check decoding
    std::string token{"abcdefghijklmnopqrstuvwxyz"};
    res.Token(token);
    assert(res.Token() == token);

    for (size_t i = 0; i < 128; i++) assert(buf[i] == 0);
    for (size_t i = 128 + res.EncodedLength(); i < 1024; i++)
      assert(buf[i] == 0);
  }

  // Ensure the message can fit in the passed buffer.
  {
    constexpr size_t kInvalidLength = SubmissionResponse::EncodedLength() - 1;
    char buf[kInvalidLength];
    bool ok{false};
    try {
      SubmissionResponse req{buf, kInvalidLength, 0};
    } catch (const std::exception& e) {
      ok = true;
    }
    assert(ok);
  }

  // Ensure token bytes are all zeroes if token is empty
  {
    constexpr size_t kLength = SubmissionResponse::EncodedLength();
    char buf[kLength];
    memset(buf, 'a', kLength);
    SubmissionResponse res{buf, kLength, 0};

    res.Token("");

    size_t start = SubmissionResponse::TokenEncodingOffset();
    size_t end = start + SubmissionResponse::TokenEncodingLength();
    for (size_t i = 0; i < start; i++) assert(buf[i] == 'a');
    for (size_t i = start; i < end - 1; i++) assert(buf[i] == 0);
    for (size_t i = end; i < kLength; i++) assert(buf[i] == 'a');
  }

  // Ensure token is truncated if it exceeds its max length.
  {
    constexpr size_t kLength = SubmissionResponse::EncodedLength() * 2;
    char buf[kLength];
    memset(buf, 0, kLength);
    SubmissionResponse res{buf, kLength, 0};

    res.Token(std::string(SubmissionResponse::TokenEncodingLength() * 2, 'a'));

    size_t start = SubmissionResponse::TokenEncodingOffset();
    size_t end = start + SubmissionResponse::TokenEncodingLength();
    for (size_t i = 0; i < start; i++) assert(buf[i] == 0);
    for (size_t i = start; i < end - 1; i++) assert(buf[i] == 'a');
    for (size_t i = end; i < kLength; i++) assert(buf[i] == 0);
  }

  std::cout << "TestSubmissionResponse done." << std::endl;
}

void TestLogoutRequest() {
  assert(LogoutRequest::EncodedLength() + MsgHeader::EncodedLength() == 13);

  // Ensure encoding/decoding works.
  {
    char buf[1024];
    memset(buf, 0, 1024);
    LogoutRequest req{buf, 1024, 128};

    assert((void*)req.Buffer() == (void*)buf);
    assert(req.BufferLength() == 1024);
    assert(req.BufferOffset() == 128);
    assert(req.EncodedLength() == 0);

    assert(req.MsgType() == 'O');

    for (size_t i = 0; i < 1024; i++) assert(buf[i] == 0);
  }

  std::cout << "TestLogoutRequest done." << std::endl;
}

void TestLogoutResponse() {
  assert(LogoutResponse::EncodedLength() + MsgHeader::EncodedLength() == 45);

  // Ensure encoding/decoding works.
  {
    char buf[1024];
    memset(buf, 0, 1024);
    LogoutResponse res{buf, 1024, 128};

    assert((void*)res.Buffer() == (void*)buf);
    assert(res.BufferLength() == 1024);
    assert(res.BufferOffset() == 128);
    assert(res.EncodedLength() == 32);

    assert(res.ReasonEncodingLength() == 32);
    assert(res.ReasonEncodingOffset() == 0);

    assert(res.MsgType() == 'G');

    // Encode and check decoding
    res.Reason("no reason");
    assert(res.Reason() == "no reason");

    for (size_t i = 0; i < 128; i++) assert(buf[i] == 0);
    for (size_t i = 128 + res.EncodedLength(); i < 1024; i++)
      assert(buf[i] == 0);
  }

  // Ensure the message can fit in the passed buffer.
  {
    constexpr size_t kInvalidLength = LogoutResponse::EncodedLength() - 1;
    char buf[kInvalidLength];
    bool ok{false};
    try {
      LogoutResponse req{buf, kInvalidLength, 0};
    } catch (const std::exception& e) {
      ok = true;
    }
    assert(ok);
  }

  // Ensure reason bytes are all zeroes if reason is empty
  {
    constexpr size_t kLength = LogoutResponse::EncodedLength();
    char buf[kLength];
    memset(buf, 'a', kLength);
    LogoutResponse res{buf, kLength, 0};

    res.Reason("");

    size_t start = LogoutResponse::ReasonEncodingOffset();
    size_t end = start + LogoutResponse::ReasonEncodingLength();
    for (size_t i = 0; i < start; i++) assert(buf[i] == 'a');
    for (size_t i = start; i < end - 1; i++) assert(buf[i] == 0);
    for (size_t i = end; i < kLength; i++) assert(buf[i] == 'a');
  }

  // Ensure reason is truncated if it exceeds its max length.
  {
    constexpr size_t kLength = LogoutResponse::EncodedLength() * 2;
    char buf[kLength];
    memset(buf, 0, kLength);
    LogoutResponse res{buf, kLength, 0};

    res.Reason(std::string(LogoutResponse::ReasonEncodingLength() * 2, 'a'));

    size_t start = LogoutResponse::ReasonEncodingOffset();
    size_t end = start + LogoutResponse::ReasonEncodingLength();
    for (size_t i = 0; i < start; i++) assert(buf[i] == 0);
    for (size_t i = start; i < end - 1; i++) assert(buf[i] == 'a');
    for (size_t i = end; i < kLength; i++) assert(buf[i] == 0);
  }

  std::cout << "TestLogoutResponse done." << std::endl;
}

void TestTcp() {
  auto server_runner = [&]() {
    try {
      Resolver resolver{};
      auto addrs = resolver.Resolve("localhost", 8080);
      assert(addrs.size() > 0);
      TcpServer tcp_server{addrs[0]};
      auto tcp_stream = tcp_server.Accept();

      const char* to_write = "12345678";
      for (size_t i = 0; i < 8; i++) {
        tcp_stream.WriteExact(to_write + i, 1);
      }
    } catch (std::exception& e) {
      std::cout << e.what() << std::endl;
    }
  };
  std::thread server_thread{server_runner};

  sleep(1);  // allow the server to come online

  auto client_runner = [&]() {
    try {
      Resolver resolver{};
      auto addrs = resolver.Resolve("localhost", 8080);
      assert(addrs.size() > 0);
      TcpClient tcp_client{};
      auto tcp_stream = tcp_client.Connect(addrs[0]);
      char buf[128];
      tcp_stream.ReadExact(buf, 8);
      assert(strcmp(buf, "12345678") == 0);
    } catch (std::exception& e) {
      std::cout << e.what() << std::endl;
    }
  };
  std::thread client_thread{client_runner};

  client_thread.join();
  server_thread.join();

  std::cout << "TestTcp done." << std::endl;
}

void TestResolver() {
  Resolver resolver{};

  auto results1 = resolver.Resolve("localhost", 8080);
  assert(results1.size() == 1);
  assert(results1[0].port == 8080);
  results1[0].addr_str = "test";

  auto results2 = resolver.Resolve("localhost", 9090);
  assert(results2.size() == 1);
  assert(results2[0].port == 9090);

  assert(results1[0].addr_str == "test");
  assert(results2[0].addr_str == "127.0.0.1");

  std::cout << "TestResolver done." << std::endl;
}

void TestTimestamp() {
  std::cout << EpochNanos() << std::endl;
  std::cout << "TestTimestamp done." << std::endl;
}

void TestChecksum() {
  // Just the header
  {
    char buf[128];
    MsgHeader header{buf, 128, 0};

    header.MsgType('A')
        .MsgLen(header.EncodedLength())
        .Timestamp(EpochNanos())
        .Checksum(0);
    assert(header.Checksum() == 0);

    Checksum(buf, header.EncodedLength(), header);
    assert(header.Checksum() != 0);

    Checksum(buf, header.EncodedLength(), header);
    assert(header.Checksum() == 0);
  }

  // Submission request
  {
    char buf[1024];
    MsgHeader header{buf, 1024, 0};
    SubmissionRequest req{buf, 1024, header.EncodedLength()};
    auto len = header.EncodedLength() + req.EncodedLength();

    header.MsgType(req.MsgType())
        .MsgLen(len)
        .Timestamp(EpochNanos())
        .Checksum(0);
    req.Name("Sergiu Marin")
        .Email("sergiu4096@gmail.com")
        .Repo("https://github.com/sergiu128/vitorian-challenge");

    assert(header.Checksum() == 0);

    Checksum(buf, len, header);
    assert(header.Checksum() != 0);

    Checksum(buf, len, header);
    assert(header.Checksum() == 0);
  }

  std::cout << "TestChecksum done." << std::endl;
}

// Runs the server in another thread and returns it. Callers should join that
// thread.
//
// Callers must specify what should the server do (if anything) through the
// todo_fn. RunServerSuccessful is an example of a todo_fn.
std::thread RunServerThread(const char* addr, int port,
                            std::function<void(TcpStream&)> todo_fn) {
  std::atomic<bool> ready{false};
  auto proto_server = [&]() {
    Resolver resolver{};
    auto addrs = resolver.Resolve(addr, port);
    assert(addrs.size() > 0);

    TcpServer server{addrs[0]};
    ready.store(true);

    TcpStream stream{server.Accept()};
    todo_fn(stream);
  };
  std::thread proto_server_runner{proto_server};

  while (!ready.load())
    ;

  return proto_server_runner;
}

// Runs the full server sequence such that the client logs in, gets a token and
// then logs out.
void RunServerSuccessful(TcpStream& stream) {
  char buf[1024];
  MsgHeader header{buf, 1024, 0};

  // read login request and write login response
  {
    stream.ReadExact(buf, header.EncodedLength());
    assert(header.MsgType() == 'L');
    assert(header.MsgLen() == 109);
    auto time_diff = EpochNanos() - header.Timestamp();
    assert(0 < time_diff && time_diff < 1'000'000'000);

    LoginRequest req{buf, 1024, header.EncodedLength()};
    stream.ReadExact(buf + header.EncodedLength(), req.EncodedLength());
    assert(req.User() == "sergiu4096@gmail.com");
    assert(req.Password() == "pwd123");

    LoginResponse res{buf, 1024, header.EncodedLength()};
    auto len = header.EncodedLength() + res.EncodedLength();
    header.MsgType(res.MsgType())
        .MsgLen(len)
        .Timestamp(EpochNanos())
        .Checksum(0);
    res.Code('Y').Reason("");
    Checksum(buf, len, header);
    stream.WriteExact(buf, len);
  }

  // read submission request and write submission response
  {
    stream.ReadExact(buf, header.EncodedLength());
    assert(header.MsgType() == 'S');
    assert(header.MsgLen() == 205);
    auto time_diff = EpochNanos() - header.Timestamp();
    assert(0 < time_diff && time_diff < 1'000'000'000);

    SubmissionRequest req{buf, 1024, header.EncodedLength()};
    stream.ReadExact(buf + header.EncodedLength(), req.EncodedLength());
    assert(req.Name() == "Sergiu Marin");
    assert(req.Email() == "sergiu4096@gmail.com");
    assert(req.Repo() == "https://github.com/sergiu128/vitorian-challenge");

    SubmissionResponse res{buf, 1024, header.EncodedLength()};
    auto len = header.EncodedLength() + res.EncodedLength();
    header.MsgType(res.MsgType())
        .MsgLen(len)
        .Timestamp(EpochNanos())
        .Checksum(0);
    res.Token("token123");
    Checksum(buf, len, header);
    stream.WriteExact(buf, len);
  }

  // read logout request and write logout response
  {
    stream.ReadExact(buf, header.EncodedLength());
    assert(header.MsgType() == 'O');
    assert(header.MsgLen() == 13);
    auto time_diff = EpochNanos() - header.Timestamp();
    assert(0 < time_diff && time_diff < 1'000'000'000);

    // logout request has no payload, it's just the header

    LogoutResponse res{buf, 1024, header.EncodedLength()};
    auto len = header.EncodedLength() + res.EncodedLength();
    header.MsgType(res.MsgType())
        .MsgLen(len)
        .Timestamp(EpochNanos())
        .Checksum(0);
    Checksum(buf, len, header);
    stream.WriteExact(buf, len);
  }

  sleep(1);
}

void TestProtoClient1() {
  std::cout << "--- ensure we throw on a wrong addr ---" << std::endl;
  {
    Resolver resolver{};
    auto addrs = resolver.Resolve("127.0.0.2", 8080);
    assert(addrs.size() > 0);

    bool thrown{};
    try {
      ProtoClient client{};
      assert(client.RunOne(addrs[0]) == false);
    } catch (std::exception& e) {
      std::cout << e.what() << std::endl;
      thrown = true;
    }
    assert(thrown);
  }

  std::cout << "--- ensure try multiple addresses ---" << std::endl;
  {
    Resolver resolver{};

    auto invalid = resolver.Resolve("127.0.0.2", 8080);
    assert(invalid.size() == 1);

    auto valid = resolver.Resolve("localhost", 8080);
    assert(valid.size() == 1);

    std::vector<Resolver::Addr> addrs{invalid[0], valid[0]};

    auto thread = RunServerThread("localhost", 8080, RunServerSuccessful);

    bool thrown{};
    try {
      ProtoClient client{};
      assert(client.Run(addrs));
    } catch (std::exception& e) {
      std::cout << e.what() << std::endl;
      thrown = true;
    }
    thread.join();
    assert(thrown == false);
  }

  std::cout << "TestProtoClient1 done." << std::endl;
}

void TestProtoClient2() {
  const char* addr = "localhost";
  int port = 8088;

  std::cout << "--- server closes immediately ---" << std::endl;
  // server closes the connection immediately
  {
    auto thread = RunServerThread(addr, port, [](TcpStream&) {});
    ProtoClient client{};
    client.Run(addr, port);
    thread.join();
    assert(client.Token() == "");
  }

  std::cout << "--- login_req -> logout_req with wrong checksum ---"
            << std::endl;
  {
    auto thread = RunServerThread(addr, port, [](TcpStream& stream) {
      char buf[1024];

      MsgHeader header{buf, 1024, 0};
      stream.ReadExact(buf, header.EncodedLength());
      assert(header.MsgType() == 'L');
      assert(header.MsgLen() == 109);
      auto time_diff = EpochNanos() - header.Timestamp();
      assert(0 < time_diff && time_diff < 1'000'000'000);

      LoginRequest req{buf, 1024, header.EncodedLength()};
      stream.ReadExact(buf + header.EncodedLength(), req.EncodedLength());
      assert(req.User() == "sergiu4096@gmail.com");
      assert(req.Password() == "pwd123");

      LogoutResponse res{buf, 1024, header.EncodedLength()};
      auto len = header.EncodedLength() + res.EncodedLength();
      header.MsgType(res.MsgType()).MsgLen(len).Timestamp(0).Checksum(0);
      res.Reason("test");

      stream.WriteExact(buf, len);

      sleep(1);
    });
    ProtoClient client{};
    client.Run(addr, port);
    thread.join();
    assert(client.Token() == "");
  }

  std::cout << "--- login_req -> logout_req with right checksum ---"
            << std::endl;
  {
    auto thread = RunServerThread(addr, port, [](TcpStream& stream) {
      char buf[1024];

      MsgHeader header{buf, 1024, 0};
      stream.ReadExact(buf, header.EncodedLength());
      assert(header.MsgType() == 'L');
      assert(header.MsgLen() == 109);
      auto time_diff = EpochNanos() - header.Timestamp();
      assert(0 < time_diff && time_diff < 1'000'000'000);

      LoginRequest req{buf, 1024, header.EncodedLength()};
      stream.ReadExact(buf + header.EncodedLength(), req.EncodedLength());
      assert(req.User() == "sergiu4096@gmail.com");
      assert(req.Password() == "pwd123");

      LogoutResponse res{buf, 1024, header.EncodedLength()};
      auto len = header.EncodedLength() + res.EncodedLength();
      header.MsgType(res.MsgType()).MsgLen(len).Timestamp(0).Checksum(0);
      res.Reason("test");
      Checksum(buf, len, header);

      stream.WriteExact(buf, len);

      sleep(1);
    });
    ProtoClient client{};
    client.Run(addr, port);
    thread.join();
    assert(client.Token() == "");
  }

  std::cout << "--- login,submit,logout successully ---" << std::endl;
  {
    auto thread = RunServerThread(addr, port, RunServerSuccessful);
    ProtoClient client{};
    client.Run(addr, port);
    thread.join();
    assert(client.Token() == "token123");
  }

  std::cout << "TestProtoClient2 done." << std::endl;
}

int main() {
  TestMsgHeader();
  TestLoginRequest();
  TestLoginResponse();
  TestSubmissionRequest();
  TestSubmissionResponse();
  TestLogoutRequest();
  TestLogoutResponse();
  TestTcp();
  TestResolver();
  TestTimestamp();
  TestChecksum();
  TestProtoClient1();
  TestProtoClient2();

  std::cout << "Bye." << std::endl;
}
