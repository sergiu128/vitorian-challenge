#include "proto_client.hpp"

#include <iostream>
#include <stdexcept>

#include "checksum.hpp"
#include "msg_header.hpp"
#include "msg_login_request.hpp"
#include "msg_login_response.hpp"
#include "msg_logout_request.hpp"
#include "msg_logout_response.hpp"
#include "msg_submission_request.hpp"
#include "msg_submission_response.hpp"
#include "util.hpp"

bool ProtoClient::Run(const char* addr, int port) {
  std::vector<Resolver::Addr> addrs{};
  try {
    addrs = resolver_.Resolve(addr, port);
    if (addrs.size() == 0) {
      std::cout << "ERR(proto-client) could not resolve addr=" << addr
                << " port=" << port << std::endl;
      throw std::runtime_error("could not resolve passed addr/port");
    }
  } catch (std::exception& e) {
    std::cout << "(proto-client) could not resolve addr=" << addr
              << " err=" << e.what() << std::endl;
    return false;
  }

  std::cout << "(proto-client) addr=" << addr << " resolved to " << addrs.size()
            << " addresses" << std::endl;

  for (const auto& resolved_addr : addrs) {
    std::cout << "(proto-client) trying on " << resolved_addr.addr_str
              << std::endl;

    try {
      if (RunOne(resolved_addr)) return true;
    } catch (std::exception& e) {
      std::cout << "(proto-client) could not run on addr="
                << resolved_addr.addr_str << "(" << addr << ")"
                << " err=" << e.what() << std::endl;
    }
  }
  return false;
}

bool ProtoClient::RunOne(const Resolver::Addr& addr) {
  token_ = "";

  std::cout << "(proto-client) connecting to addr=" << addr.addr_str
            << " port=" << addr.port << std::endl;

  TcpStream stream{client_.Connect(addr)};

  std::cout << "(proto-client) connected to addr=" << addr.addr_str
            << " port=" << addr.port << std::endl;

  char buf[1024];

  // Write LoginRequest
  {
    MsgHeader header{buf, 1024, 0};
    LoginRequest req{buf, 1024, header.EncodedLength()};

    auto len = header.EncodedLength() + req.EncodedLength();

    header.MsgType(req.MsgType())
        .MsgLen(len)
        .Timestamp(EpochNanos())
        .Checksum(0);
    req.User("sergiu4096@gmail.com").Password("pwd123");
    Checksum(buf, len, header);

    stream.WriteExact(buf, len);

    std::cout << "(proto-client) sent login request" << std::endl;
  }

  // Read response: LoginResponse (success) or LogoutResponse (failure)
  {
    MsgHeader header{buf, 1024, 0};
    stream.ReadExact(buf, header.EncodedLength());

    if (header.MsgType() == LoginResponse::MsgType()) {
      LoginResponse res{buf, 1024, header.EncodedLength()};
      auto len = header.EncodedLength() + res.EncodedLength();

      stream.ReadExact(buf + header.EncodedLength(), res.EncodedLength());

      Checksum(buf, len, header);
      if (header.Checksum() != 0) {
        throw std::runtime_error(
            "(proto-client) invalid login response checksum");
      }

      if (res.Code() == 'Y') {
        std::cout << "(proto-client) logged in code=" << res.Code()
                  << " reason=" << res.Reason() << std::endl;
      } else {
        throw std::runtime_error("(proto-client) could not login reason=" +
                                 res.Reason());
      }
    } else if (header.MsgType() == LogoutResponse::MsgType()) {
      LogoutResponse res{buf, 1024, header.EncodedLength()};
      auto len = header.EncodedLength() + res.EncodedLength();

      stream.ReadExact(buf + header.EncodedLength(), res.EncodedLength());

      Checksum(buf, len, header);
      if (header.Checksum() != 0) {
        throw std::runtime_error(
            "(proto-client) invalid logout response checksum");
      }

      std::cout << "(proto-client) received logout response reason="
                << res.Reason() << std::endl;

      return false;
    } else {
      throw std::runtime_error("unexpected response");
    }
  }

  // Write SubmissionRequest
  {
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
    Checksum(buf, len, header);

    stream.WriteExact(buf, len);

    std::cout << "(proto-client) sent submission request" << std::endl;
  }

  // Read response: SubmissionResponse (success) or LogoutResponse (failure)
  {
    MsgHeader header{buf, 1024, 0};
    stream.ReadExact(buf, header.EncodedLength());

    if (header.MsgType() == SubmissionResponse::MsgType()) {
      SubmissionResponse res{buf, 1024, header.EncodedLength()};
      auto len = header.EncodedLength() + res.EncodedLength();

      stream.ReadExact(buf + header.EncodedLength(), res.EncodedLength());

      Checksum(buf, len, header);
      if (header.Checksum() != 0) {
        throw std::runtime_error(
            "(proto-client) invalid submission response checksum");
      }

      std::cout << "(proto-client) received submission response token="
                << res.Token() << std::endl;

      token_ = res.Token();
    } else if (header.MsgType() == LogoutResponse::MsgType()) {
      LogoutResponse res{buf, 1024, header.EncodedLength()};
      auto len = header.EncodedLength() + res.EncodedLength();

      stream.ReadExact(buf + header.EncodedLength(), res.EncodedLength());

      Checksum(buf, len, header);
      if (header.Checksum() != 0) {
        throw std::runtime_error(
            "(proto-client) invalid logout response checksum");
      }

      std::cout << "(proto-client) received logout response reason="
                << res.Reason() << std::endl;

      return false;
    } else {
      throw std::runtime_error("unexpected response");
    }
  }

  // Write LogoutRequest
  {
    MsgHeader header{buf, 1024, 0};
    LogoutRequest req{buf, 1024, header.EncodedLength()};

    auto len = header.EncodedLength() + req.EncodedLength();

    header.MsgType(req.MsgType())
        .MsgLen(len)
        .Timestamp(EpochNanos())
        .Checksum(0);
    Checksum(buf, len, header);

    stream.WriteExact(buf, len);

    std::cout << "(proto-client) sent logout request" << std::endl;
  }

  // Read logout response
  {
    MsgHeader header{buf, 1024, 0};
    LogoutResponse res{buf, 1024, header.EncodedLength()};
    auto len = header.EncodedLength() + res.EncodedLength();

    stream.ReadExact(buf, len);

    Checksum(buf, len, header);
    if (header.Checksum() != 0) {
      throw std::runtime_error(
          "(proto-client) invalid logout response checksum");
    }

    std::cout << "(proto-client) received logout response reason="
              << res.Reason() << std::endl;
  }

  return true;
}
