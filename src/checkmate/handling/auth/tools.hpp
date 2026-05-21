#pragma once

#include <array>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <format>
#include <memory>
#include <optional>
#include <random>
#include <string>

#include <pbkdf2_sha256.h>
#include <sqlite3.h>

namespace checkmate::handling::auth::utils {

struct SqliteDeleter {
  void operator()(sqlite3* db) const {
    if (db)
      sqlite3_close(db);
  }
};

using DbPtr = std::unique_ptr<sqlite3, SqliteDeleter>;

[[nodiscard]] inline std::string DatabasePath() {
  const char* database_url = std::getenv("DATABASE_URL");
  if (!database_url)
    return "checkmate.sqlite3";

  std::string path = database_url;
  if (path.starts_with("sqlite:"))
    return path.substr(7);
  return path;
}

[[nodiscard]] inline DbPtr ConnectDb() {
  sqlite3* db = nullptr;
  if (sqlite3_open(DatabasePath().c_str(), &db) != SQLITE_OK) {
    sqlite3_close(db);
    return nullptr;
  }
  return DbPtr{db};
}

[[nodiscard]] inline std::string HashPassword(const std::string& password) {
  std::mt19937_64 rng{std::random_device{}()};
  std::string salt = std::format("{:016x}{:016x}", rng(), rng());

  constexpr std::uint32_t rounds = 600000;
  constexpr std::uint32_t dklen = SHA256_DIGESTLEN;
  std::array<std::uint8_t, dklen> dk{};
  HMAC_SHA256_CTX ctx;
  pbkdf2_sha256(&ctx, reinterpret_cast<const std::uint8_t*>(password.data()),
                static_cast<std::uint32_t>(password.size()),
                reinterpret_cast<const std::uint8_t*>(salt.data()),
                static_cast<std::uint32_t>(salt.size()), rounds, dk.data(),
                dklen);

  std::string hashed = std::format("pbkdf2_sha256${}", rounds);
  hashed += '$' + salt + '$';
  for (const auto byte : dk)
    hashed += std::format("{:02x}", byte);
  return hashed;
}

[[nodiscard]] inline bool VerifyPassword(const std::string& password,
                                         const std::string& stored) {
  auto p1 = stored.find('$');
  if (p1 == std::string::npos)
    return false;
  auto p2 = stored.find('$', p1 + 1);
  if (p2 == std::string::npos)
    return false;
  auto p3 = stored.find('$', p2 + 1);
  if (p3 == std::string::npos)
    return false;

  if (stored.substr(0, p1) != "pbkdf2_sha256")
    return false;

  const auto rounds = static_cast<std::uint32_t>(
      std::stoul(stored.substr(p1 + 1, p2 - p1 - 1)));
  const std::string salt = stored.substr(p2 + 1, p3 - p2 - 1);
  const std::string hex_dk = stored.substr(p3 + 1);

  constexpr std::uint32_t dklen = SHA256_DIGESTLEN;
  if (hex_dk.size() != dklen * 2)
    return false;

  std::array<std::uint8_t, dklen> dk{};
  HMAC_SHA256_CTX ctx;
  pbkdf2_sha256(&ctx, reinterpret_cast<const std::uint8_t*>(password.data()),
                static_cast<std::uint32_t>(password.size()),
                reinterpret_cast<const std::uint8_t*>(salt.data()),
                static_cast<std::uint32_t>(salt.size()), rounds, dk.data(),
                dklen);

  for (std::uint32_t i = 0; i < dklen; ++i) {
    char buf[3];
    std::snprintf(buf, sizeof(buf), "%02x", dk[i]);
    if (hex_dk[2 * i] != buf[0] || hex_dk[2 * i + 1] != buf[1])
      return false;
  }
  return true;
}

struct UserInfo {
  std::string username;
  std::string role;
};

[[nodiscard]] inline std::optional<UserInfo> FetchUserInfo(
    sqlite3* db,
    const std::string& token) {
  sqlite3_stmt* stmt = nullptr;
  const char* sql = R"SQL(
SELECT u.username, u.role
  FROM auth_session s
  JOIN auth_user u ON u.username = s.username
 WHERE s.key = ?
   AND s.revoked = 0
   AND s.expires_at > CURRENT_TIMESTAMP
   AND u.enabled = 1
)SQL";

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    return std::nullopt;

  sqlite3_bind_text(stmt, 1, token.c_str(), -1, SQLITE_TRANSIENT);

  std::optional<UserInfo> info;
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    info = UserInfo{
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)),
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)),
    };
  }

  sqlite3_finalize(stmt);
  return info;
}

}  // namespace checkmate::handling::auth::utils
