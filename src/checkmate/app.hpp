#pragma once

#include <crow_all.h>

#include "checkmate/handling/auth/middlewares.hpp"

namespace checkmate {

using App = crow::App<handling::auth::middlewares::LogInRequired,
                      handling::auth::middlewares::GuardRequired,
                      handling::auth::middlewares::AdminRequired>;

}  // namespace checkmate
