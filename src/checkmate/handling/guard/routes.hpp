#pragma once

#include <crow_all.h>

#include "checkmate/app.hpp"
#include "checkmate/handling/auth/utils.hpp"

namespace checkmate::handling::guard {

void AddRoutes(checkmate::App& app);

<<<<<<< Updated upstream
crow::response IndexGet(const auth::utils::UserInfo& info);
crow::response DashboardGet(const auth::utils::UserInfo& info);
crow::response EmployeesGet(const crow::request& req,
                            const auth::utils::UserInfo& info);
=======
crow::response IndexGet(const crow::request& req);
crow::response DashboardGet(const crow::request& req);
crow::response EmployeesGet(const crow::request& req);
crow::response SearchGet(const crow::request& req);
>>>>>>> Stashed changes

}  // namespace checkmate::handling::guard
