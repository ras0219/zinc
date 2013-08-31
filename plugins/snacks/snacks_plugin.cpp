#include <cassert>
#include <zinc_plugin>
#include <vector>

using namespace std;
using namespace pnp;

namespace SnacksPlugin
{
  static std::vector<const char*> botsnacks =
  { "&botsnack", "~jumpsnack", ".botsnack", "+botsnack",
    "~botsnack", "^botsnack" };

  static unsigned int snacks = 2;

  void snacktime(Context* ctx, str_t) {
    if (snacks < 2) {
      ctx->vtable->reply(ctx, "Sorry, I don't have enough snacks.");
      return;
    } else {
      snacks -= 2;
    }
      
    ctx->vtable->reply(ctx, botsnacks[rand() % botsnacks.size()]);
    ctx->vtable->reply(ctx, botsnacks[rand() % botsnacks.size()]);
  }

  void botsnack(Context* ctx, str_t) {
    snacks++;
    std::stringstream ss;
    ss << "Thanks for the snack! I now have " << snacks << " snack";
    if (snacks > 1) ss << "s."; else ss << ".";
    ctx->vtable->reply(ctx, ss.str().c_str());
    return;
  }

  void install(PluginHost* host) {
    host->vtable->register_command(host, "snacktime", &snacktime);
    host->vtable->register_command(host, "botsnack", &botsnack);
  }

  static str_t plugin_name = "SnacksMachine";
  static SemanticVersion plugin_version = { 0, 0, 0 };

  static PluginBase plugbase = {
    &install,
    nullptr, // uninstall
    nullptr, // transfer
    plugin_name,
    plugin_version
  };
};

PluginBase* get_plugin(std::size_t n) {
  assert(n == 0);
  return &SnacksPlugin::plugbase;
}

pnp_module_t pnp_module =
{
  CUR_ZINC_VERSION,             // req_zinc_version
  nullptr,                      // init
  nullptr,                      // destroy

  1,                            // num_exported_plugins
  get_plugin                    // get_plugin
};
