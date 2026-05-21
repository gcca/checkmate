#include "checkmate/handling/admin/routes.hpp"

#include "checkmate/handling/auth/middlewares.hpp"

namespace cm_auth = checkmate::handling::auth;

namespace checkmate::handling::admin {

namespace {

auto col(sqlite3_stmt* s, int i) {
  auto p = sqlite3_column_text(s, i);
  return p ? reinterpret_cast<const char*>(p) : "";
}

crow::json::wvalue::list FetchDocumentTypes(sqlite3* db,
                                            std::string_view selected = {}) {
  crow::json::wvalue::list types;
  sqlite3_stmt* stmt = nullptr;
  if (sqlite3_prepare_v2(
          db,
          "SELECT name, display_name FROM cm_document_type ORDER BY display_name",
          -1, &stmt, nullptr) != SQLITE_OK)
    return types;

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    crow::json::wvalue t;
    std::string name = col(stmt, 0);
    const bool is_selected = !selected.empty() && name == selected;
    t["name"] = name;
    t["display_name"] = col(stmt, 1);
    if (is_selected)
      t["selected"] = true;
    types.push_back(std::move(t));
  }
  sqlite3_finalize(stmt);
  return types;
}

crow::response EmployeeListFragment(sqlite3* db, const std::string& username,
                                    const std::string& search = "") {
  const bool filtering = !search.empty();

  sqlite3_stmt* stmt = nullptr;
  const char* sql_all = R"SQL(
SELECT e.rowid, e.name, e.document_number, t.display_name, e.created_at
  FROM cm_employee e
  JOIN cm_document_type t ON t.name = e.document_type
 ORDER BY e.name
)SQL";
  const char* sql_filtered = R"SQL(
SELECT e.rowid, e.name, e.document_number, t.display_name, e.created_at
  FROM cm_employee e
  JOIN cm_document_type t ON t.name = e.document_type
 WHERE e.name LIKE ? OR e.document_number LIKE ?
 ORDER BY e.name
)SQL";

  if (sqlite3_prepare_v2(db, filtering ? sql_filtered : sql_all, -1, &stmt,
                         nullptr) != SQLITE_OK)
    return crow::response{503};

  if (filtering) {
    const std::string pattern = "%" + search + "%";
    sqlite3_bind_text(stmt, 1, pattern.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, pattern.c_str(), -1, SQLITE_TRANSIENT);
  }

  crow::json::wvalue::list employees;
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    crow::json::wvalue e;
    e["rowid"] = sqlite3_column_int(stmt, 0);
    e["name"] = col(stmt, 1);
    e["document_number"] = col(stmt, 2);
    e["document_type"] = col(stmt, 3);
    e["created_at"] = col(stmt, 4);
    employees.push_back(std::move(e));
  }
  sqlite3_finalize(stmt);

  crow::mustache::context ctx;
  ctx["username"] = username;
  ctx["search"] = search;
  ctx["employees"] = std::move(employees);
  return crow::response{
      crow::mustache::load("handling/admin/templates/employee/list.html")
          .render(ctx)};
}

}  // namespace

crow::response IndexGet(const crow::request& req) {
  cm_auth::utils::UserInfo info;
  if (auto err = cm_auth::middlewares::RoleRequired(req, "admin", info))
    return std::move(*err);

  crow::mustache::context ctx;
  ctx["username"] = info.username;
  return crow::response{
      crow::mustache::load("handling/admin/templates/index.html").render(ctx)};
}

crow::response DashboardGet(const crow::request& req) {
  cm_auth::utils::UserInfo info;
  if (auto err = cm_auth::middlewares::RoleRequired(req, "admin", info))
    return std::move(*err);

  crow::mustache::context ctx;
  ctx["username"] = info.username;
  return crow::response{
      crow::mustache::load("handling/admin/templates/dashboard.html")
          .render(ctx)};
}

crow::response EmployeeListGet(const crow::request& req) {
  cm_auth::utils::UserInfo info;
  if (auto err = cm_auth::middlewares::RoleRequired(req, "admin", info))
    return std::move(*err);

  const char* s = req.url_params.get("s");
  const std::string search = s ? s : "";

  auto db = cm_auth::utils::ConnectDb();
  if (!db)
    return crow::response{503};

  return EmployeeListFragment(db.get(), info.username, search);
}

crow::response EmployeeCreateGet(const crow::request& req) {
  cm_auth::utils::UserInfo info;
  if (auto err = cm_auth::middlewares::RoleRequired(req, "admin", info))
    return std::move(*err);

  auto db = cm_auth::utils::ConnectDb();
  if (!db)
    return crow::response{503};

  crow::mustache::context ctx;
  ctx["username"] = info.username;
  ctx["document_types"] = FetchDocumentTypes(db.get());
  return crow::response{
      crow::mustache::load("handling/admin/templates/employee/create.html")
          .render(ctx)};
}

crow::response EmployeeCreatePost(const crow::request& req) {
  cm_auth::utils::UserInfo info;
  if (auto err = cm_auth::middlewares::RoleRequired(req, "admin", info))
    return std::move(*err);

  const auto params = req.get_body_params();
  const char* name_p = params.get("name");
  const char* doc_type_p = params.get("document_type");
  const char* doc_num_p = params.get("document_number");

  if (!name_p || !doc_type_p || !doc_num_p)
    return crow::response{400};

  auto db = cm_auth::utils::ConnectDb();
  if (!db)
    return crow::response{503};

  sqlite3_stmt* stmt = nullptr;
  const char* sql =
      "INSERT INTO cm_employee (name, document_type, document_number) "
      "VALUES (?, ?, ?)";
  if (sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK)
    return crow::response{503};

  sqlite3_bind_text(stmt, 1, name_p, -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 2, doc_type_p, -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 3, doc_num_p, -1, SQLITE_TRANSIENT);

  const bool ok = sqlite3_step(stmt) == SQLITE_DONE;
  sqlite3_finalize(stmt);

  if (!ok)
    return crow::response{409};

  return EmployeeListFragment(db.get(), info.username);
}

crow::response EmployeeDetailsGet(const crow::request& req, int rowid) {
  cm_auth::utils::UserInfo info;
  if (auto err = cm_auth::middlewares::RoleRequired(req, "admin", info))
    return std::move(*err);

  auto db = cm_auth::utils::ConnectDb();
  if (!db)
    return crow::response{503};

  sqlite3_stmt* stmt = nullptr;
  const char* sql = R"SQL(
SELECT e.rowid, e.name, e.document_number, t.display_name, e.document_type, e.created_at
  FROM cm_employee e
  JOIN cm_document_type t ON t.name = e.document_type
 WHERE e.rowid = ?
)SQL";

  if (sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK)
    return crow::response{503};

  sqlite3_bind_int(stmt, 1, rowid);

  if (sqlite3_step(stmt) != SQLITE_ROW) {
    sqlite3_finalize(stmt);
    return crow::response{404};
  }

  crow::mustache::context ctx;
  ctx["username"] = info.username;
  ctx["rowid"] = sqlite3_column_int(stmt, 0);
  ctx["name"] = col(stmt, 1);
  ctx["document_number"] = col(stmt, 2);
  ctx["document_type_display"] = col(stmt, 3);
  ctx["document_type"] = col(stmt, 4);
  ctx["created_at"] = col(stmt, 5);
  sqlite3_finalize(stmt);

  return crow::response{
      crow::mustache::load("handling/admin/templates/employee/details.html")
          .render(ctx)};
}

crow::response EmployeeUpdateGet(const crow::request& req, int rowid) {
  cm_auth::utils::UserInfo info;
  if (auto err = cm_auth::middlewares::RoleRequired(req, "admin", info))
    return std::move(*err);

  auto db = cm_auth::utils::ConnectDb();
  if (!db)
    return crow::response{503};

  sqlite3_stmt* stmt = nullptr;
  const char* sql =
      "SELECT rowid, name, document_number, document_type FROM cm_employee "
      "WHERE rowid = ?";

  if (sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK)
    return crow::response{503};

  sqlite3_bind_int(stmt, 1, rowid);

  if (sqlite3_step(stmt) != SQLITE_ROW) {
    sqlite3_finalize(stmt);
    return crow::response{404};
  }

  crow::mustache::context ctx;
  ctx["username"] = info.username;
  ctx["rowid"] = sqlite3_column_int(stmt, 0);
  ctx["name"] = col(stmt, 1);
  ctx["document_number"] = col(stmt, 2);
  const std::string current_doc_type = col(stmt, 3);
  sqlite3_finalize(stmt);

  ctx["document_types"] = FetchDocumentTypes(db.get(), current_doc_type);

  return crow::response{
      crow::mustache::load("handling/admin/templates/employee/update.html")
          .render(ctx)};
}

crow::response EmployeeUpdatePost(const crow::request& req, int rowid) {
  cm_auth::utils::UserInfo info;
  if (auto err = cm_auth::middlewares::RoleRequired(req, "admin", info))
    return std::move(*err);

  const auto params = req.get_body_params();
  const char* name_p = params.get("name");
  const char* doc_type_p = params.get("document_type");
  const char* doc_num_p = params.get("document_number");

  if (!name_p || !doc_type_p || !doc_num_p)
    return crow::response{400};

  auto db = cm_auth::utils::ConnectDb();
  if (!db)
    return crow::response{503};

  sqlite3_stmt* stmt = nullptr;
  const char* sql =
      "UPDATE cm_employee SET name=?, document_type=?, document_number=? "
      "WHERE rowid=?";
  if (sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK)
    return crow::response{503};

  sqlite3_bind_text(stmt, 1, name_p, -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 2, doc_type_p, -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 3, doc_num_p, -1, SQLITE_TRANSIENT);
  sqlite3_bind_int(stmt, 4, rowid);

  const bool ok = sqlite3_step(stmt) == SQLITE_DONE;
  sqlite3_finalize(stmt);

  if (!ok)
    return crow::response{409};

  return EmployeeListFragment(db.get(), info.username);
}

crow::response EmployeeDeletePost(const crow::request& req, int rowid) {
  cm_auth::utils::UserInfo info;
  if (auto err = cm_auth::middlewares::RoleRequired(req, "admin", info))
    return std::move(*err);

  auto db = cm_auth::utils::ConnectDb();
  if (!db)
    return crow::response{503};

  sqlite3_stmt* stmt = nullptr;
  if (sqlite3_prepare_v2(db.get(), "DELETE FROM cm_employee WHERE rowid=?", -1,
                         &stmt, nullptr) != SQLITE_OK)
    return crow::response{503};

  sqlite3_bind_int(stmt, 1, rowid);
  sqlite3_step(stmt);
  sqlite3_finalize(stmt);

  return EmployeeListFragment(db.get(), info.username);
}

void AddRoutes(crow::SimpleApp& app) {
  CROW_ROUTE(app, "/checkmate/admin")
      .methods(crow::HTTPMethod::GET)(IndexGet);

  CROW_ROUTE(app, "/checkmate/admin/dashboard")
      .methods(crow::HTTPMethod::GET)(DashboardGet);

  CROW_ROUTE(app, "/checkmate/admin/employee")
      .methods(crow::HTTPMethod::GET)(EmployeeListGet);

  CROW_ROUTE(app, "/checkmate/admin/employee/create")
      .methods(crow::HTTPMethod::GET)(EmployeeCreateGet);

  CROW_ROUTE(app, "/checkmate/admin/employee/create")
      .methods(crow::HTTPMethod::POST)(EmployeeCreatePost);

  CROW_ROUTE(app, "/checkmate/admin/employee/<int>/details")
      .methods(crow::HTTPMethod::GET)(EmployeeDetailsGet);

  CROW_ROUTE(app, "/checkmate/admin/employee/<int>/update")
      .methods(crow::HTTPMethod::GET)(EmployeeUpdateGet);

  CROW_ROUTE(app, "/checkmate/admin/employee/<int>/update")
      .methods(crow::HTTPMethod::POST)(EmployeeUpdatePost);

  CROW_ROUTE(app, "/checkmate/admin/employee/<int>/delete")
      .methods(crow::HTTPMethod::POST)(EmployeeDeletePost);
}

}  // namespace checkmate::handling::admin
