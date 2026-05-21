#pragma once

#include <crow_all.h>

namespace checkmate::handling::admin {

void AddRoutes(crow::SimpleApp& app);

crow::response IndexGet(const crow::request& req);
crow::response DashboardGet(const crow::request& req);

crow::response EmployeeListGet(const crow::request& req);
crow::response EmployeeCreateGet(const crow::request& req);
crow::response EmployeeCreatePost(const crow::request& req);
crow::response EmployeeDetailsGet(const crow::request& req, int rowid);
crow::response EmployeeUpdateGet(const crow::request& req, int rowid);
crow::response EmployeeUpdatePost(const crow::request& req, int rowid);
crow::response EmployeeDeletePost(const crow::request& req, int rowid);

}  // namespace checkmate::handling::admin
