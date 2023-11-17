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

// Runs the client login,submit,logout sequence on the IPs resolved from the
// given address and port.
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

  return Run(addrs);
}

// Runs the client login,submit,logout sequence on the set of resolved IPs.
bool ProtoClient::Run(const std::vector<Resolver::Addr>& addrs) {
  for (const auto& resolved_addr : addrs) {
    std::cout << std::endl;
    std::cout << "(proto-client) trying on " << resolved_addr.addr_str
              << std::endl;

    try {
      if (RunOne(resolved_addr)) return true;
    } catch (std::exception& e) {
      std::cout << "(proto-client) could not run on addr="
                << resolved_addr.addr_str << " err=" << e.what() << std::endl;
    }
  }
  return false;
}

void ProtoClient::WriteLoginRequest(TcpStream& stream) {
  std::cout << "(proto-client) sending login request" << std::endl;

  MsgHeader header{buf_, 1024, 0};
  LoginRequest req{buf_, 1024, header.EncodedLength()};

  auto len = header.EncodedLength() + req.EncodedLength();

  header.MsgType(req.MsgType()).MsgLen(len).Timestamp(EpochNanos()).Checksum(0);
  req.User("sergiu4096@gmail.com").Password("pwd123");
  Checksum(buf_, len, header);

  stream.WriteExact(buf_, len);

  std::cout << "(proto-client) sent " << header << " " << req << std::endl;
}

// Try to read the login response. If successful, returns true.
bool ProtoClient::ReadLoginResponse(TcpStream& stream) {
  for (size_t i = 0; i < kMaxRetriesOnWrongChecksum; i++) {
    std::cout << "(proto-client) trying to read login response" << std::endl;

    // read the header, figure out how many bytes the body is and then read it
    stream.ReadExact(buf_, header_.EncodedLength());

    if (header_.MsgLen() < header_.EncodedLength()) {
      throw std::runtime_error("msg length smaller than header length");
    }

    stream.ReadExact(buf_ + header_.EncodedLength(),
                     header_.MsgLen() - header_.EncodedLength());

    std::cout << "(proto-client) received " << header_ << std::endl;

    Checksum(buf_, header_.MsgLen(), header_);
    if (header_.Checksum() != 0) {
      std::cout << "(proto-client) invalid checksum for msg_type="
                << header_.MsgType() << ", retrying..." << std::endl;
      continue;
    }

    if (header_.MsgType() == LoginResponse::MsgType()) {
      LoginResponse res{buf_, 1024, header_.EncodedLength()};

      std::cout << "(proto-client) received " << res << std::endl;

      if (res.Code() == 'Y') {
        std::cout << "(proto-client) logged in successfully code=" << res.Code()
                  << " reason=" << res.Reason() << std::endl;

        return true;
      } else {
        throw std::runtime_error("(proto-client) could not login reason=" +
                                 res.Reason());
      }
    } else if (header_.MsgType() == LogoutResponse::MsgType()) {
      LogoutResponse res{buf_, 1024, header_.EncodedLength()};
      std::cout << "(proto-client) received " << res << std::endl;
      return false;
    } else {
      throw std::runtime_error("unexpected response");
    }
  }

  return false;
}

void ProtoClient::WriteSubmissionRequest(TcpStream& stream) {
  SubmissionRequest req{buf_, 1024, header_.EncodedLength()};

  auto len = header_.EncodedLength() + req.EncodedLength();

  header_.MsgType(req.MsgType())
      .MsgLen(len)
      .Timestamp(EpochNanos())
      .Checksum(0);
  req.Name("Sergiu Marin")
      .Email("sergiu4096@gmail.com")
      .Repo("https://github.com/sergiu128/vitorian-challenge");
  Checksum(buf_, len, header_);

  stream.WriteExact(buf_, len);

  std::cout << "(proto-client) sent " << header_ << " " << req << std::endl;
}

// Try to read the submission response. If successful, returns true.
bool ProtoClient::ReadSubmissionResponse(TcpStream& stream) {
  for (size_t i = 0; i < kMaxRetriesOnWrongChecksum; i++) {
    std::cout << "(proto-client) trying to read submission response"
              << std::endl;

    // read the header, figure out how many bytes the body is and then read it
    stream.ReadExact(buf_, header_.EncodedLength());

    if (header_.MsgLen() < header_.EncodedLength()) {
      throw std::runtime_error("msg length smaller than header length");
    }

    stream.ReadExact(buf_ + header_.EncodedLength(),
                     header_.MsgLen() - header_.EncodedLength());

    std::cout << "(proto-client) received " << header_ << std::endl;

    Checksum(buf_, header_.MsgLen(), header_);
    if (header_.Checksum() != 0) {
      std::cout << "(proto-client) invalid checksum for msg_type="
                << header_.MsgType() << ", retrying..." << std::endl;
      continue;
    }

    if (header_.MsgType() == SubmissionResponse::MsgType()) {
      SubmissionResponse res{buf_, 1024, header_.EncodedLength()};
      std::cout << "(proto-client) received " << res << std::endl;
      std::cout << "(proto-client) received submission response token="
                << res.Token() << std::endl;
      token_ = res.Token();
      return true;
    } else if (header_.MsgType() == LogoutResponse::MsgType()) {
      LogoutResponse res{buf_, 1024, header_.EncodedLength()};
      std::cout << "(proto-client) received " << res << std::endl;
      return false;
    } else {
      throw std::runtime_error("unexpected response");
    }
  }

  return false;
}

void ProtoClient::WriteLogoutRequest(TcpStream& stream) {
  LogoutRequest req{buf_, 1024, header_.EncodedLength()};

  auto len = header_.EncodedLength() + req.EncodedLength();

  header_.MsgType(req.MsgType())
      .MsgLen(len)
      .Timestamp(EpochNanos())
      .Checksum(0);
  Checksum(buf_, len, header_);

  stream.WriteExact(buf_, len);

  std::cout << "(proto-client) sent " << header_ << " " << req << std::endl;
}

// Try to read the logout response. If successful, returns true.
bool ProtoClient::ReadLogoutResponse(TcpStream& stream) {
  for (size_t i = 0; i < kMaxRetriesOnWrongChecksum; i++) {
    std::cout << "(proto-client) trying to read logout response" << std::endl;

    // read the header, figure out how many bytes the body is and then read it
    stream.ReadExact(buf_, header_.EncodedLength());

    if (header_.MsgLen() < header_.EncodedLength()) {
      throw std::runtime_error("msg length smaller than header length");
    }

    stream.ReadExact(buf_ + header_.EncodedLength(),
                     header_.MsgLen() - header_.EncodedLength());

    std::cout << "(proto-client) received " << header_ << std::endl;

    Checksum(buf_, header_.MsgLen(), header_);
    if (header_.Checksum() != 0) {
      std::cout << "(proto-client) invalid checksum for msg_type="
                << header_.MsgType() << ", retrying..." << std::endl;
      continue;
    }

    LogoutResponse res{buf_, 1024, header_.EncodedLength()};
    std::cout << "(proto-client) logged out reason=" << res.Reason()
              << std::endl;
    return true;
  }

  return false;
}

// Given the address, runs the client login,submit,logout sequence. Throws if
// anything goes wrong.
bool ProtoClient::RunOne(const Resolver::Addr& addr) {
  token_ = "";

  std::cout << "(proto-client) connecting to addr=" << addr.addr_str
            << " port=" << addr.port << std::endl;

  TcpStream stream{client_.Connect(addr)};

  std::cout << "(proto-client) connected to addr=" << addr.addr_str
            << " port=" << addr.port << std::endl;

  WriteLoginRequest(stream);
  if (!ReadLoginResponse(stream)) return false;

  WriteSubmissionRequest(stream);
  if (!ReadSubmissionResponse(stream)) return false;

  WriteLogoutRequest(stream);
  if (!ReadLogoutResponse(stream)) return false;

  return true;
}
