#include <unistd.h>

#include <cassert>
#include <iostream>
#include <memory>
#include <thread>

#include "msg_header.hpp"
#include "msg_login_request.hpp"
#include "msg_login_response.hpp"
#include "msg_logout_request.hpp"
#include "msg_logout_response.hpp"
#include "msg_submission_request.hpp"
#include "msg_submission_response.hpp"
#include "resolver.hpp"
#include "tcp_client.hpp"
#include "tcp_server.hpp"

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
    assert(buf[end - 1] == '\0');
    for (size_t i = end; i < kLength; i++) assert(buf[i] == 0);
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
    assert(buf[end - 1] == '\0');
    for (size_t i = end; i < kLength; i++) assert(buf[i] == 0);
  }

  std::cout << "TestLoginRequest done." << std::endl;
}

void TestLoginResponse() {
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
    assert(buf[end - 1] == '\0');
    for (size_t i = end; i < kLength; i++) assert(buf[i] == 0);
  }

  std::cout << "TestLoginResponse done." << std::endl;
}

void TestSubmissionRequest() {
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
    assert(buf[end - 1] == '\0');
    for (size_t i = end; i < kLength; i++) assert(buf[i] == 0);
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
    assert(buf[end - 1] == '\0');
    for (size_t i = end; i < kLength; i++) assert(buf[i] == 0);
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
    assert(buf[end - 1] == '\0');
    for (size_t i = end; i < kLength; i++) assert(buf[i] == 0);
  }

  std::cout << "TestSubmissionRequest done." << std::endl;
}

void TestSubmissionResponse() {
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
    assert(buf[end - 1] == '\0');
    for (size_t i = end; i < kLength; i++) assert(buf[i] == 0);
  }

  std::cout << "TestSubmissionResponse done." << std::endl;
}

void TestLogoutRequest() {
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
    assert(buf[end - 1] == '\0');
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

  std::cout << "Bye." << std::endl;
}
