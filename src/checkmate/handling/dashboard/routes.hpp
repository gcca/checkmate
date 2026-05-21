#pragma once

#include <crow_all.h>

namespace checkmate::handling::dashboard {

void AddRoutes(crow::SimpleApp& app);

crow::response IndexGet(const crow::request& req);

}  // namespace checkmate::handling::dashboard
