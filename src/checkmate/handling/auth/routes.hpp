#pragma once

#include <crow_all.h>

namespace checkmate::handling::auth {

void AddRoutes(crow::SimpleApp& app);

crow::response SignInGet();
crow::response SignInPost(const crow::request& req);
crow::response SignOutPost(const crow::request& req);

}  // namespace checkmate::handling::auth
