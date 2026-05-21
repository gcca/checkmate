#pragma once

#include <crow_all.h>

#include "checkmate/app.hpp"
#include "checkmate/handling/auth/utils.hpp"

namespace checkmate::handling::dashboard {

void AddRoutes(checkmate::App& app);

crow::response IndexGet(const auth::utils::UserInfo& info);

}  // namespace checkmate::handling::dashboard
