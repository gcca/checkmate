#pragma once

#include <optional>
#include <string>
#include <string_view>

#include <crow_all.h>
#include <sqlite3.h>

#include "checkmate/handling/auth/tools.hpp"

namespace checkmate::handling::auth::utils {

[[nodiscard]] inline std::optional<std::string> Authenticate(
    sqlite3* db,
    const std::string& username,
    const std::string& password) {
  sqlite3_stmt* stmt = nullptr;
  const char* sql = R"SQL(
SELECT password
  FROM auth_user
 WHERE username = ?
   AND enabled = 1
)SQL";

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    return std::nullopt;
  }

  sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);

  std::optional<std::string> result;
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    const auto* stored =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    if (stored && VerifyPassword(password, stored))
      result = username;
  }

  sqlite3_finalize(stmt);
  return result;
}

[[nodiscard]] inline std::string GenerateToken() {
  std::mt19937_64 rng{std::random_device{}()};
  std::string token = "checkmate-v1_";
  for (int i = 0; i < 4; ++i)
    token += std::format("{:016x}", rng());
  return token;
}

[[nodiscard]] inline std::optional<std::string> CreateSession(
    sqlite3* db,
    const std::string& username) {
  const std::string token = GenerateToken();
  sqlite3_stmt* stmt = nullptr;
  const char* sql = R"SQL(
INSERT INTO auth_session (key, username, expires_at)
VALUES (?, ?, datetime('now', '+1 month'))
)SQL";

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    return std::nullopt;
  }

  sqlite3_bind_text(stmt, 1, token.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 2, username.c_str(), -1, SQLITE_TRANSIENT);

  const bool ok = sqlite3_step(stmt) == SQLITE_DONE;
  sqlite3_finalize(stmt);
  if (!ok)
    return std::nullopt;

  sqlite3_stmt* update = nullptr;
  const char* update_sql =
      "UPDATE auth_user SET last_logged_at = CURRENT_TIMESTAMP WHERE username "
      "= ?";
  if (sqlite3_prepare_v2(db, update_sql, -1, &update, nullptr) == SQLITE_OK) {
    sqlite3_bind_text(update, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(update);
  }
  sqlite3_finalize(update);
  return token;
}

[[nodiscard]] inline bool RevokeSession(sqlite3* db, const std::string& token) {
  sqlite3_stmt* stmt = nullptr;
  const char* sql = "UPDATE auth_session SET revoked = 1 WHERE key = ?";

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    return false;
  }

  sqlite3_bind_text(stmt, 1, token.c_str(), -1, SQLITE_TRANSIENT);
  const bool ok = sqlite3_step(stmt) == SQLITE_DONE && sqlite3_changes(db) == 1;
  sqlite3_finalize(stmt);
  return ok;
}

[[nodiscard]] inline std::optional<std::string> CookieValue(
    const crow::request& req,
    std::string_view name) {
  const auto& header = req.get_header_value("Cookie");
  std::size_t pos = 0;
  while (pos < header.size()) {
    while (pos < header.size() && (header[pos] == ' ' || header[pos] == ';'))
      ++pos;
    const std::size_t eq = header.find('=', pos);
    if (eq == std::string::npos)
      break;
    const std::size_t end = header.find(';', eq + 1);
    if (std::string_view(header.data() + pos, eq - pos) == name) {
      return header.substr(eq + 1,
                           end == std::string::npos ? end : end - eq - 1);
    }
    if (end == std::string::npos)
      break;
    pos = end + 1;
  }
  return std::nullopt;
}

[[nodiscard]] inline crow::response Redirect(const std::string& location) {
  crow::response res;
  res.moved(location);
  return res;
}

inline void SetSessionCookie(crow::response& res, const std::string& token) {
  res.add_header(
      "Set-Cookie",
      "token=" + token + "; Path=/checkmate; HttpOnly; SameSite=Strict");
}

inline void ClearSessionCookie(crow::response& res) {
  res.add_header("Set-Cookie",
                 "token=; Path=/checkmate; HttpOnly; SameSite=Strict; "
                 "Max-Age=0");
}

}  // namespace checkmate::handling::auth::utils
