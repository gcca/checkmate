#pragma once

#include <crow_all.h>

#include "checkmate/app.hpp"

namespace checkmate::handling::auth {

void AddRoutes(checkmate::App& app);

crow::response SignInGet();
crow::response SignInPost(const crow::request& req);
crow::response SignOutPost(const crow::request& req);

}  // namespace checkmate::handling::auth
