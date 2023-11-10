#include <cassert>
#include <iostream>

#include "msg_header.hpp"
#include "msg_login_request.hpp"
#include "msg_login_response.hpp"
#include "msg_logout_request.hpp"
#include "msg_logout_response.hpp"
#include "msg_submission_request.hpp"
#include "msg_submission_response.hpp"

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
    assert(buf[LoginRequest::UserEncodingOffset() +
               LoginRequest::UserEncodingLength() - 1] == '\0');
  }

  // Ensure password is truncated if it exceeds its max length.
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
    assert(buf[LoginRequest::PasswordEncodingOffset() +
               LoginRequest::PasswordEncodingLength() - 1] == '\0');
  }

  std::cout << "TestLoginRequest done." << std::endl;
}

void TestLoginResponse() {
  // Ensure encoding/decoding works.
  {
    char buf[128];
    LoginResponse res{buf, 128, 0};

    assert((void*)res.Buffer() == (void*)buf);
    assert(res.BufferLength() == 128);
    assert(res.BufferOffset() == 0);
    assert(res.EncodedLength() == 33);

    assert(res.CodeEncodingLength() == 1);
    assert(res.CodeEncodingOffset() == 0);

    assert(res.ReasonEncodingLength() == 32);
    assert(res.ReasonEncodingOffset() == 1);

    assert(res.MsgType() == 'E');

    // Encode and check decoding
    char code = 'Y';

    const char* reason = "no reason";
    size_t reason_length = strlen(reason);

    res.Code(code).Reason([&](char* slice, size_t len) {
      assert(len == res.ReasonEncodingLength() - 1);
      assert((void*)slice == (void*)((char*)buf + res.ReasonEncodingOffset()));
      memcpy(slice, reason, reason_length);
      return reason_length;
    });
    assert(res.Code() == 'Y');
    assert(strcmp(res.Reason(), reason) == 0);
  }

  // Ensure the message can fit in the passed buffer.
  {
    constexpr size_t InvalidLength = LoginResponse::EncodedLength() - 1;
    char buf[InvalidLength];
    bool ok{false};
    try {
      LoginResponse req{buf, InvalidLength, 0};
    } catch (const std::exception& e) {
      ok = true;
    }
    assert(ok);
  }

  // Ensure reason is truncated if it exceeds its max length.
  {
    constexpr size_t Length = LoginResponse::EncodedLength() * 2;
    char buf[Length];
    memset(buf, 0, Length);
    LoginResponse res{buf, Length, 0};

    res.Reason([&](char* slice, size_t max_length) {
      for (size_t i = 0; i < max_length + 2; i++) slice[i] = 'a';
      return max_length + 2;
    });
    for (size_t i = LoginResponse::ReasonEncodingOffset();
         i < LoginResponse::ReasonEncodingLength() - 1; i++) {
      assert(buf[i] == 'a');
    }
    assert(buf[LoginResponse::ReasonEncodingOffset() +
               LoginResponse::ReasonEncodingLength() - 1] == '\0');
  }

  std::cout << "TestLoginResponse done." << std::endl;
}

void TestSubmissionRequest() {
  // Ensure encoding/decoding works.
  {
    char buf[256];
    SubmissionRequest req{buf, 256, 0};

    assert((void*)req.Buffer() == (void*)buf);
    assert(req.BufferLength() == 256);
    assert(req.BufferOffset() == 0);
    assert(req.EncodedLength() == 3 * 64);

    assert(req.NameEncodingLength() == 64);
    assert(req.NameEncodingOffset() == 0);

    assert(req.EmailEncodingLength() == 64);
    assert(req.EmailEncodingOffset() == 64);

    assert(req.RepoEncodingLength() == 64);
    assert(req.RepoEncodingOffset() == 128);

    assert(req.MsgType() == 'S');

    // Encode and check decoding
    const char* name = "Sergiu Marin";
    size_t name_length = strlen(name);

    const char* email = "sergiu4096@gmail.com";
    size_t email_length = strlen(email);

    const char* repo = "https://github.com/sergiu128/vitorian-challenge";
    size_t repo_length = strlen(repo);

    req.Name([&](char* slice, size_t len) {
         assert(len == req.NameEncodingLength() - 1);
         assert((void*)slice == (void*)((char*)buf + req.NameEncodingOffset()));
         memcpy(slice, name, name_length);
         return name_length;
       })
        .Email([&](char* slice, size_t len) {
          assert(len == req.EmailEncodingLength() - 1);
          assert((void*)slice ==
                 (void*)((char*)buf + req.EmailEncodingOffset()));
          memcpy(slice, email, email_length);
          return email_length;
        })
        .Repo([&](char* slice, size_t len) {
          assert(len == req.RepoEncodingLength() - 1);
          assert((void*)slice ==
                 (void*)((char*)buf + req.RepoEncodingOffset()));
          memcpy(slice, repo, repo_length);
          return repo_length;
        });
    assert(strcmp(req.Name(), name) == 0);
    assert(strcmp(req.Email(), email) == 0);
    assert(strcmp(req.Repo(), repo) == 0);
  }

  // Ensure the message can fit in the passed buffer.
  {
    constexpr size_t InvalidLength = SubmissionRequest::EncodedLength() - 1;
    char buf[InvalidLength];
    bool ok{false};
    try {
      SubmissionRequest req{buf, InvalidLength, 0};
    } catch (const std::exception& e) {
      ok = true;
    }
    assert(ok);
  }

  // Ensure name is truncated if it exceeds its max length.
  {
    constexpr size_t Length = SubmissionRequest::EncodedLength() * 2;
    char buf[Length];
    memset(buf, 0, Length);
    SubmissionRequest req{buf, Length, 0};

    req.Name([&](char* slice, size_t max_length) {
      for (size_t i = 0; i < max_length + 2; i++) slice[i] = 'a';
      return max_length + 2;
    });
    for (size_t i = SubmissionRequest::NameEncodingOffset();
         i < SubmissionRequest::NameEncodingLength() - 1; i++) {
      assert(buf[i] == 'a');
    }
    assert(buf[SubmissionRequest::NameEncodingOffset() +
               SubmissionRequest::NameEncodingLength() - 1] == '\0');
  }

  // Ensure email is truncated if it exceeds its max length.
  {
    constexpr size_t Length = SubmissionRequest::EncodedLength() * 2;
    char buf[Length];
    memset(buf, 0, Length);
    SubmissionRequest req{buf, Length, 0};

    req.Email([&](char* slice, size_t max_length) {
      for (size_t i = 0; i < max_length + 2; i++) slice[i] = 'a';
      return max_length + 2;
    });
    for (size_t i = SubmissionRequest::EmailEncodingOffset();
         i < SubmissionRequest::EmailEncodingLength() - 1; i++) {
      assert(buf[i] == 'a');
    }
    assert(buf[SubmissionRequest::EmailEncodingOffset() +
               SubmissionRequest::EmailEncodingLength() - 1] == '\0');
  }

  // Ensure repo is truncated if it exceeds its max length.
  {
    constexpr size_t Length = SubmissionRequest::EncodedLength() * 2;
    char buf[Length];
    memset(buf, 0, Length);
    SubmissionRequest req{buf, Length, 0};

    req.Repo([&](char* slice, size_t max_length) {
      for (size_t i = 0; i < max_length + 2; i++) slice[i] = 'a';
      return max_length + 2;
    });
    for (size_t i = SubmissionRequest::RepoEncodingOffset();
         i < SubmissionRequest::RepoEncodingLength() - 1; i++) {
      assert(buf[i] == 'a');
    }
    assert(buf[SubmissionRequest::RepoEncodingOffset() +
               SubmissionRequest::RepoEncodingLength() - 1] == '\0');
  }

  std::cout << "TestSubmissionRequest done." << std::endl;
}

void TestSubmissionResponse() {
  // Ensure encoding/decoding works.
  {
    char buf[128];
    SubmissionResponse res{buf, 128, 0};

    assert((void*)res.Buffer() == (void*)buf);
    assert(res.BufferLength() == 128);
    assert(res.BufferOffset() == 0);
    assert(res.EncodedLength() == 32);

    assert(res.TokenEncodingLength() == 32);
    assert(res.TokenEncodingOffset() == 0);

    assert(res.MsgType() == 'R');

    // Encode and check decoding
    const char* token = "abcdefghijklmnopqrstuvwxyz";
    size_t token_length = strlen(token);

    res.Token([&](char* slice, size_t len) {
      assert(len == res.TokenEncodingLength() - 1);
      assert((void*)slice == (void*)((char*)buf + res.TokenEncodingOffset()));
      memcpy(slice, token, token_length);
      return token_length;
    });
    assert(strcmp(res.Token(), token) == 0);
  }

  // Ensure the message can fit in the passed buffer.
  {
    constexpr size_t InvalidLength = SubmissionResponse::EncodedLength() - 1;
    char buf[InvalidLength];
    bool ok{false};
    try {
      SubmissionResponse req{buf, InvalidLength, 0};
    } catch (const std::exception& e) {
      ok = true;
    }
    assert(ok);
  }

  // Ensure token is truncated if it exceeds its max length.
  {
    constexpr size_t Length = SubmissionResponse::EncodedLength() * 2;
    char buf[Length];
    memset(buf, 0, Length);
    SubmissionResponse res{buf, Length, 0};

    res.Token([&](char* slice, size_t max_length) {
      for (size_t i = 0; i < max_length + 2; i++) slice[i] = 'a';
      return max_length + 2;
    });
    for (size_t i = SubmissionResponse::TokenEncodingOffset();
         i < SubmissionResponse::TokenEncodingLength() - 1; i++) {
      assert(buf[i] == 'a');
    }
    assert(buf[SubmissionResponse::TokenEncodingOffset() +
               SubmissionResponse::TokenEncodingLength() - 1] == '\0');
  }

  std::cout << "TestSubmissionResponse done." << std::endl;
}

void TestLogoutRequest() {
  // Ensure encoding/decoding works.
  {
    char buf[128];
    LogoutRequest req{buf, 128, 0};

    assert((void*)req.Buffer() == (void*)buf);
    assert(req.BufferLength() == 128);
    assert(req.BufferOffset() == 0);
    assert(req.EncodedLength() == 0);

    assert(req.MsgType() == 'O');
  }

  std::cout << "TestLogoutRequest done." << std::endl;
}

void TestLogoutResponse() {
  // Ensure encoding/decoding works.
  {
    char buf[128];
    LogoutResponse res{buf, 128, 0};

    assert((void*)res.Buffer() == (void*)buf);
    assert(res.BufferLength() == 128);
    assert(res.BufferOffset() == 0);
    assert(res.EncodedLength() == 32);

    assert(res.ReasonEncodingLength() == 32);
    assert(res.ReasonEncodingOffset() == 0);

    assert(res.MsgType() == 'G');

    // Encode and check decoding
    const char* reason = "no reason";
    size_t reason_length = strlen(reason);

    res.Reason([&](char* slice, size_t len) {
      assert(len == res.ReasonEncodingLength() - 1);
      assert((void*)slice == (void*)((char*)buf + res.ReasonEncodingOffset()));
      memcpy(slice, reason, reason_length);
      return reason_length;
    });
    assert(strcmp(res.Reason(), reason) == 0);
  }

  // Ensure the message can fit in the passed buffer.
  {
    constexpr size_t InvalidLength = LogoutResponse::EncodedLength() - 1;
    char buf[InvalidLength];
    bool ok{false};
    try {
      LogoutResponse req{buf, InvalidLength, 0};
    } catch (const std::exception& e) {
      ok = true;
    }
    assert(ok);
  }

  // Ensure reason is truncated if it exceeds its max length.
  {
    constexpr size_t Length = LogoutResponse::EncodedLength() * 2;
    char buf[Length];
    memset(buf, 0, Length);
    LogoutResponse res{buf, Length, 0};

    res.Reason([&](char* slice, size_t max_length) {
      for (size_t i = 0; i < max_length + 2; i++) slice[i] = 'a';
      return max_length + 2;
    });
    for (size_t i = LogoutResponse::ReasonEncodingOffset();
         i < LogoutResponse::ReasonEncodingLength() - 1; i++) {
      assert(buf[i] == 'a');
    }
    assert(buf[LogoutResponse::ReasonEncodingOffset() +
               LogoutResponse::ReasonEncodingLength() - 1] == '\0');
  }

  std::cout << "TestLogoutResponse done." << std::endl;
}

int main() {
  TestMsgHeader();
  TestLoginRequest();
  TestLoginResponse();
  TestSubmissionRequest();
  TestSubmissionResponse();
  TestLogoutRequest();
  TestLogoutResponse();

  std::cout << "Bye." << std::endl;
}
