#include <cassert>
#include <zinc_plugin>
#include <vector>
#include <zinc_interactive>

using namespace std;
using namespace zinc;

namespace SnacksPlugin
{
  static std::vector<const char*> botsnacks =
  { "&botsnack", "~jumpsnack", ".botsnack", "+botsnack",
    "~botsnack", "^botsnack" };

  static unsigned int snacks = 2;

  void snacktime(OStream::Interface* ctx, str_t) {
    if (snacks < 2) {
      ctx->vtable->send(ctx, "Sorry, I don't have enough snacks.");
      return;
    } else {
      snacks -= 2;
    }
      
    ctx->vtable->send(ctx, botsnacks[rand() % botsnacks.size()]);
    ctx->vtable->send(ctx, botsnacks[rand() % botsnacks.size()]);
  }

  void botsnack(OStream::Interface* ctx, str_t) {
    snacks++;
    std::stringstream ss;
    ss << "Thanks for the snack! I now have " << snacks << " snack";
    if (snacks > 1) ss << "s."; else ss << ".";
    ctx->vtable->send(ctx, ss.str().c_str());
    return;
  }

  struct InstallInterfaces {
    Interactive::Interface* i;
  };

  void install(Interface** argv) {
    InstallInterfaces* ii = (InstallInterfaces*)(argv);
    auto i = ii->i;
    i->vtable->register_command(i, "snacktime", &snacktime);
    i->vtable->register_command(i, "botsnack", &botsnack);
  }

  static str_t plugin_name = "SnacksMachine";
  static SemanticVersion plugin_version = { 0, 1, 0 };

  InterfaceRequest requests[] = {
    Interactive::Request::required
  };

  static Plugin plugbase = {
    &install,
    nullptr, // uninstall
    nullptr, // transfer
    plugin_name,
    plugin_version,

    1,
    requests
  };
};

Plugin* get_plugin(std::size_t n) {
  assert(n == 0);
  return &SnacksPlugin::plugbase;
}

pnp_module_t pnp_module =
{
  CUR_ZINC_VERSION,             // req_zinc_version
  1,                            // num_exported_plugins
  get_plugin                    // get_plugin
};
