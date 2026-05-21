#include <CLI11.hpp>
#include <print>
#include <string>

#include "checkmate/handling/auth/tools.hpp"

int main(int argc, char* argv[]) {
  CLI::App app{"Create a checkmate user"};

  std::string username;
  std::string password;
  std::string role = "guard";

  app.add_option("-u,--username", username, "Username")->required();
  app.add_option("-p,--password", password, "Password")->required();
  app.add_option("-r,--role", role, "Role: root, admin, guard")
      ->check(CLI::IsMember({"root", "admin", "guard"}));

  CLI11_PARSE(app, argc, argv);

  auto db = checkmate::handling::auth::utils::ConnectDb();
  if (!db) {
    std::println(stderr, "Error opening database");
    return 1;
  }

  sqlite3_stmt* stmt = nullptr;
  const char* sql = R"SQL(
INSERT INTO auth_user (username, password, role)
VALUES (?, ?, ?)
)SQL";

  if (sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
    std::println(stderr, "Error preparing statement: {}", sqlite3_errmsg(db.get()));
    return 1;
  }

  const std::string hashed =
      checkmate::handling::auth::utils::HashPassword(password);

  sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 2, hashed.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 3, role.c_str(), -1, SQLITE_TRANSIENT);

  const int rc = sqlite3_step(stmt);
  if (rc != SQLITE_DONE) {
    std::println(stderr, "Error creating user: {}", sqlite3_errmsg(db.get()));
    sqlite3_finalize(stmt);
    return 1;
  }

  sqlite3_finalize(stmt);
  std::println("User '{}' created successfully", username);
  return 0;
}
