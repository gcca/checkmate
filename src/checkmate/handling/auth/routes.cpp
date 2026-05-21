#include "checkmate/handling/auth/routes.hpp"

#include "checkmate/handling/auth/utils.hpp"

namespace checkmate::handling::auth {

static crow::response RenderSignIn(std::string_view error = {}) {
  crow::mustache::context ctx;
  if (!error.empty())
    ctx["error"] = std::string(error);
  return crow::response{
      crow::mustache::load("handling/auth/templates/signin.html").render(ctx)};
}

crow::response SignInGet() {
  return RenderSignIn();
}

crow::response SignInPost(const crow::request& req) {
  const auto params = req.get_body_params();
  const char* username_param = params.get("username");
  const char* password_param = params.get("password");

  if (!username_param || !password_param)
    return RenderSignIn("Usuario y contraseña son requeridos.");

  auto db = utils::ConnectDb();
  if (!db)
    return RenderSignIn("Error al conectar con la base de datos.");

  const std::string username = username_param;
  const std::string password = password_param;
  const auto auth_username = utils::Authenticate(db.get(), username, password);
  if (!auth_username)
    return RenderSignIn("Usuario o contraseña incorrectos.");

  const auto token = utils::CreateSession(db.get(), *auth_username);
  if (!token)
    return crow::response{500, "Session creation failed"};

  auto res = utils::Redirect("/checkmate/dashboard");
  utils::SetSessionCookie(res, *token);
  return res;
}

crow::response SignOutPost(const crow::request& req) {
  const auto token = utils::CookieValue(req, "token");
  if (token) {
    if (auto db = utils::ConnectDb()) {
      (void)utils::RevokeSession(db.get(), *token);
    }
  }

  auto res = utils::Redirect("/checkmate/auth/signin");
  utils::ClearSessionCookie(res);
  return res;
}

void AddRoutes(crow::SimpleApp& app) {
  crow::mustache::set_global_base("src/checkmate");

  CROW_ROUTE(app, "/checkmate/auth/signin")
      .methods(crow::HTTPMethod::GET)(SignInGet);

  CROW_ROUTE(app, "/checkmate/auth/signin")
      .methods(crow::HTTPMethod::POST)(SignInPost);

  CROW_ROUTE(app, "/checkmate/auth/signout")
      .methods(crow::HTTPMethod::POST)(SignOutPost);
}

}  // namespace checkmate::handling::auth
