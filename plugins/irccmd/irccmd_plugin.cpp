#include <iostream>
#include <cassert>
#include <zinc_plugin>

using namespace std;
using namespace pnp;

namespace IrcCmdPlugin
{
  void joinchannel(Context* ctx, str_t remainder) {
    if (remainder == nullptr or *remainder == 0) {
      ctx->vtable->reply(ctx, "Usage: -join <channel>");
      return;
    }
    ctx->vtable->irc_join(ctx, remainder);
  }

  void install(PluginHost* host) {
    host->vtable->register_command(host, "join", &joinchannel);
  }

  static str_t plugin_name = "IrcCmd";
  static SemanticVersion plugin_version = { 0, 1, 0 };

  static PluginBase plugbase = {
    &install,
    plugin_name,
    plugin_version
  };
};

PluginBase* get_plugin(std::size_t n) {
  assert(n == 0);
  return &IrcCmdPlugin::plugbase;
}

pnp_module_t pnp_module =
{
  nullptr,                      // init
  1,                            // num_exported_plugins
  get_plugin,                   // get_plugin
  nullptr                       // destroy
};
