#include <crow_all.h>
#include <CLI11.hpp>

#include "checkmate/app.hpp"
#include "checkmate/handling/admin/routes.hpp"
#include "checkmate/handling/auth/routes.hpp"
#include "checkmate/handling/dashboard/routes.hpp"
#include "checkmate/handling/guard/routes.hpp"

int main(int argc, char* argv[]) {
  CLI::App app{"CheckMate — mall employee ingress control"};

  std::uint16_t port = 5571;
  app.add_option("-p,--port", port, "Listening port")->capture_default_str();

  CLI11_PARSE(app, argc, argv);

  checkmate::App server;

  CROW_ROUTE(server, "/checkmate")([] {
    crow::response res;
    res.moved("/checkmate/auth/signin");
    return res;
  });

  CROW_ROUTE(server, "/checkmate/healthcheck")([] { return "🍻"; });

  checkmate::handling::auth::AddRoutes(server);
  checkmate::handling::dashboard::AddRoutes(server);
  checkmate::handling::guard::AddRoutes(server);
  checkmate::handling::admin::AddRoutes(server);

  server.port(port).multithreaded().run();
}
