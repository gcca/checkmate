#pragma once

#include <optional>
#include <string_view>

#include <crow_all.h>

#include "checkmate/handling/auth/utils.hpp"

namespace checkmate::handling::auth::middlewares {

[[nodiscard]] inline std::optional<crow::response> LogInRequired(
    const crow::request& req,
    utils::UserInfo& out) {
  const auto token = utils::CookieValue(req, "token");
  if (!token)
    return utils::Redirect("/checkmate/auth/signin");

  auto db = utils::ConnectDb();
  if (!db)
    return utils::Redirect("/checkmate/auth/signin");

  const auto info = utils::FetchUserInfo(db.get(), *token);
  if (!info)
    return utils::Redirect("/checkmate/auth/signin");

  out = *info;
  return std::nullopt;
}

[[nodiscard]] inline std::optional<crow::response> RoleRequired(
    const crow::request& req,
    std::string_view role,
    utils::UserInfo& out) {
  if (auto err = LogInRequired(req, out))
    return err;
  if (out.role != role)
    return crow::response{403};
  return std::nullopt;
}

}  // namespace checkmate::handling::auth::middlewares
