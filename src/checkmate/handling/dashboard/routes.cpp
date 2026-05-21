#include "checkmate/handling/dashboard/routes.hpp"

namespace checkmate::handling::dashboard {

crow::response IndexGet(const auth::utils::UserInfo& info) {
  if (info.role == "guard")
    return auth::utils::Redirect("/checkmate/guard");

  if (info.role == "admin" || info.role == "root")
    return auth::utils::Redirect("/checkmate/admin");

  return auth::utils::Redirect("/checkmate/guard");
}

void AddRoutes(checkmate::App& app) {
  using auth::middlewares::LogInRequired;

  static crow::Blueprint dashboard_bp("checkmate/dashboard",
                                      ".checkmate-static", "src/checkmate");
  dashboard_bp.middlewares<checkmate::App, LogInRequired>();

  CROW_BP_ROUTE(dashboard_bp, "")
      .methods(crow::HTTPMethod::GET)([&app](const crow::request& req) {
        return IndexGet(app.get_context<LogInRequired>(req).user);
      });

  app.register_blueprint(dashboard_bp);
}

}  // namespace checkmate::handling::dashboard
