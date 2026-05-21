#pragma once

#include <crow_all.h>

namespace checkmate::handling::guard {

void AddRoutes(crow::SimpleApp& app);

crow::response IndexGet(const crow::request& req);
crow::response DashboardGet(const crow::request& req);
crow::response EmployeesGet(const crow::request& req);

}  // namespace checkmate::handling::guard
