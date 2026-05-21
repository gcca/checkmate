#pragma once

#include <optional>
#include <string>
#include <string_view>

#include <crow_all.h>

#include "checkmate/handling/auth/utils.hpp"

namespace checkmate::handling::auth::middlewares {

enum class Role {
  root,
  admin,
  guard,
};

constexpr std::string_view RoleName(Role role) {
  switch (role) {
    case Role::root:
      return "root";
    case Role::admin:
      return "admin";
    case Role::guard:
      return "guard";
  }
  return "";
}

constexpr bool RoleSatisfies(std::string_view actual, Role required) {
  if (actual == RoleName(required))
    return true;
  return required == Role::admin && actual == RoleName(Role::root);
}

inline void EndWith(crow::response& res, crow::response next) {
  res = std::move(next);
  res.end();
}

[[nodiscard]] inline std::optional<crow::response> RequireLogIn(
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

[[nodiscard]] inline std::optional<crow::response>
RequireRole(const crow::request& req, Role role, utils::UserInfo& out) {
  if (auto err = RequireLogIn(req, out))
    return err;
  if (!RoleSatisfies(out.role, role))
    return crow::response{403};
  return std::nullopt;
}

struct LogInRequired : crow::ILocalMiddleware {
  struct context {
    utils::UserInfo user;
  };

  void before_handle(crow::request& req, crow::response& res, context& ctx) {
    if (auto err = RequireLogIn(req, ctx.user))
      EndWith(res, std::move(*err));
  }

  void after_handle(crow::request&, crow::response&, context&) {}
};

template <Role RequiredRole>
struct RoleRequired : crow::ILocalMiddleware {
  struct context {
    utils::UserInfo user;
  };

  void before_handle(crow::request& req, crow::response& res, context& ctx) {
    if (auto err = RequireRole(req, RequiredRole, ctx.user))
      EndWith(res, std::move(*err));
  }

  void after_handle(crow::request&, crow::response&, context&) {}
};

using GuardRequired = RoleRequired<Role::guard>;
using AdminRequired = RoleRequired<Role::admin>;

}  // namespace checkmate::handling::auth::middlewares
