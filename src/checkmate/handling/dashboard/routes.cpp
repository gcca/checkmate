#include "checkmate/handling/dashboard/routes.hpp"

#include "checkmate/handling/auth/middlewares.hpp"
#include "checkmate/handling/auth/utils.hpp"

namespace checkmate::handling::dashboard {

crow::response IndexGet(const crow::request& req) {
  auth::utils::UserInfo info;
  if (auto err = auth::middlewares::LogInRequired(req, info))
    return std::move(*err);

  if (info.role == "guard")
    return auth::utils::Redirect("/checkmate/guard");

  if (info.role == "admin" || info.role == "root")
    return auth::utils::Redirect("/checkmate/admin");

  return auth::utils::Redirect("/checkmate/guard");
}

void AddRoutes(crow::SimpleApp& app) {
  CROW_ROUTE(app, "/checkmate/dashboard")
      .methods(crow::HTTPMethod::GET)(IndexGet);
}

}  // namespace checkmate::handling::dashboard
