#include "checkmate/handling/guard/routes.hpp"

#include "checkmate/handling/auth/middlewares.hpp"

namespace cm_auth = checkmate::handling::auth;

namespace checkmate::handling::guard {

crow::response IndexGet(const cm_auth::utils::UserInfo& info) {
  crow::mustache::context ctx;
  ctx["username"] = info.username;
  return crow::response{
      crow::mustache::load("handling/guard/templates/index.html").render(ctx)};
}

crow::response DashboardGet(const cm_auth::utils::UserInfo& info) {
  crow::mustache::context ctx;
  ctx["username"] = info.username;
  return crow::response{
      crow::mustache::load("handling/guard/templates/dashboard.html")
          .render(ctx)};
}

crow::response EmployeesGet(const crow::request& req,
                            const cm_auth::utils::UserInfo& info) {
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

<<<<<<< Updated upstream
void AddRoutes(checkmate::App& app) {
  using cm_auth::middlewares::GuardRequired;
=======
crow::response SearchGet(const crow::request& req) {
  cm_auth::utils::UserInfo info;
  if (auto err = cm_auth::middlewares::RoleRequired(req, "guard", info))
    return std::move(*err);

  const char* search_param = req.url_params.get("s");
  const std::string search = search_param ? search_param : "";

  crow::json::wvalue::list employees;
  if (!search.empty()) {
    auto db = cm_auth::utils::ConnectDb();
    if (!db)
      return crow::response{503};

    sqlite3_stmt* stmt = nullptr;
    const char* sql = R"SQL(
SELECT e.name, e.document_number, t.display_name, e.created_at
  FROM cm_employee e
  JOIN cm_document_type t ON t.name = e.document_type
 WHERE e.document_number LIKE ?
 ORDER BY e.name
)SQL";
    if (sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr) != SQLITE_OK)
      return crow::response{503};

    const std::string pattern = "%" + search + "%";
    sqlite3_bind_text(stmt, 1, pattern.c_str(), -1, SQLITE_TRANSIENT);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
      crow::json::wvalue e;
      const char* name_c = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
      std::string name = name_c ? name_c : "";
      e["name"] = name;
      e["document_number"] =
          reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
      e["document_type"] =
          reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
      e["created_at"] =
          reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
      char init = name.empty() ? '?' : static_cast<char>(std::toupper(static_cast<unsigned char>(name[0])));
      e["initials"] = std::string(1, init);
      employees.push_back(std::move(e));
    }
    sqlite3_finalize(stmt);
  }

  crow::mustache::context ctx;
  ctx["username"] = info.username;
  ctx["search"] = search;
  ctx["employees"] = std::move(employees);
  return crow::response{
      crow::mustache::load("handling/guard/templates/search.html")
          .render(ctx)};
}

void AddRoutes(crow::SimpleApp& app) {
  CROW_ROUTE(app, "/checkmate/guard").methods(crow::HTTPMethod::GET)(IndexGet);
>>>>>>> Stashed changes

  static crow::Blueprint guard_bp("checkmate/guard", ".checkmate-static",
                                  "src/checkmate");
  guard_bp.middlewares<checkmate::App, GuardRequired>();

<<<<<<< Updated upstream
  CROW_BP_ROUTE(guard_bp, "")
      .methods(crow::HTTPMethod::GET)([&app](const crow::request& req) {
        return IndexGet(app.get_context<GuardRequired>(req).user);
      });

  CROW_BP_ROUTE(guard_bp, "/dashboard")
      .methods(crow::HTTPMethod::GET)([&app](const crow::request& req) {
        return DashboardGet(app.get_context<GuardRequired>(req).user);
      });

  CROW_BP_ROUTE(guard_bp, "/employees")
      .methods(crow::HTTPMethod::GET)([&app](const crow::request& req) {
        return EmployeesGet(req, app.get_context<GuardRequired>(req).user);
      });

  app.register_blueprint(guard_bp);
=======
  CROW_ROUTE(app, "/checkmate/guard/employees")
      .methods(crow::HTTPMethod::GET)(EmployeesGet);

  CROW_ROUTE(app, "/checkmate/guard/search")
      .methods(crow::HTTPMethod::GET)(SearchGet);
>>>>>>> Stashed changes
}

}  // namespace checkmate::handling::guard
