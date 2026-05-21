#pragma once

#include <crow_all.h>

#include "checkmate/app.hpp"
#include "checkmate/handling/auth/utils.hpp"

namespace checkmate::handling::admin {

void AddRoutes(checkmate::App& app);

crow::response IndexGet(const auth::utils::UserInfo& info);
crow::response DashboardGet(const auth::utils::UserInfo& info);

crow::response EmployeeListGet(const crow::request& req,
                               const auth::utils::UserInfo& info);
crow::response EmployeeCreateGet(const auth::utils::UserInfo& info);
crow::response EmployeeCreatePost(const crow::request& req,
                                  const auth::utils::UserInfo& info);
crow::response EmployeeDetailsGet(const auth::utils::UserInfo& info, int rowid);
crow::response EmployeeUpdateGet(const auth::utils::UserInfo& info, int rowid);
crow::response EmployeeUpdatePost(const crow::request& req,
                                  const auth::utils::UserInfo& info,
                                  int rowid);
crow::response EmployeeDeletePost(const auth::utils::UserInfo& info, int rowid);

}  // namespace checkmate::handling::admin
