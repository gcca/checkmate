#include "checkmate/handling/guard/routes.hpp"

#include "checkmate/handling/auth/middlewares.hpp"

namespace cm_auth = checkmate::handling::auth;

namespace checkmate::handling::guard {

crow::response IndexGet(const crow::request& req) {
  cm_auth::utils::UserInfo info;
  if (auto err = cm_auth::middlewares::RoleRequired(req, "guard", info))
    return std::move(*err);

  crow::mustache::context ctx;
  ctx["username"] = info.username;
  return crow::response{
      crow::mustache::load("handling/guard/templates/index.html").render(ctx)};
}

crow::response DashboardGet(const crow::request& req) {
  cm_auth::utils::UserInfo info;
  if (auto err = cm_auth::middlewares::RoleRequired(req, "guard", info))
    return std::move(*err);

  crow::mustache::context ctx;
  ctx["username"] = info.username;
  return crow::response{
      crow::mustache::load("handling/guard/templates/dashboard.html")
          .render(ctx)};
}

crow::response EmployeesGet(const crow::request& req) {
  cm_auth::utils::UserInfo info;
  if (auto err = cm_auth::middlewares::RoleRequired(req, "guard", info))
    return std::move(*err);

  const char* search_param = req.url_params.get("s");
  const std::string search = search_param ? search_param : "";
  const bool filtering = !search.empty();

  auto db = cm_auth::utils::ConnectDb();
  if (!db)
    return crow::response{503};

  sqlite3_stmt* stmt = nullptr;
  const char* sql_all = R"SQL(
SELECT e.name, e.document_number, t.display_name, e.created_at
  FROM cm_employee e
  JOIN cm_document_type t ON t.name = e.document_type
 ORDER BY e.name
)SQL";
  const char* sql_filtered = R"SQL(
SELECT e.name, e.document_number, t.display_name, e.created_at
  FROM cm_employee e
  JOIN cm_document_type t ON t.name = e.document_type
 WHERE e.name LIKE ? OR e.document_number LIKE ?
 ORDER BY e.name
)SQL";

  if (sqlite3_prepare_v2(db.get(), filtering ? sql_filtered : sql_all, -1,
                         &stmt, nullptr) != SQLITE_OK)
    return crow::response{503};

  if (filtering) {
    const std::string pattern = "%" + search + "%";
    sqlite3_bind_text(stmt, 1, pattern.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, pattern.c_str(), -1, SQLITE_TRANSIENT);
  }

  crow::json::wvalue::list employees;
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    crow::json::wvalue e;
    e["name"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    e["document_number"] =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    e["document_type"] =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    e["created_at"] =
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    employees.push_back(std::move(e));
  }
  sqlite3_finalize(stmt);

  crow::mustache::context ctx;
  ctx["username"] = info.username;
  ctx["search"] = search;
  ctx["employees"] = std::move(employees);
  return crow::response{
      crow::mustache::load("handling/guard/templates/employees.html")
          .render(ctx)};
}

void AddRoutes(crow::SimpleApp& app) {
  CROW_ROUTE(app, "/checkmate/guard").methods(crow::HTTPMethod::GET)(IndexGet);

  CROW_ROUTE(app, "/checkmate/guard/dashboard")
      .methods(crow::HTTPMethod::GET)(DashboardGet);

  CROW_ROUTE(app, "/checkmate/guard/employees")
      .methods(crow::HTTPMethod::GET)(EmployeesGet);
}

}  // namespace checkmate::handling::guard
