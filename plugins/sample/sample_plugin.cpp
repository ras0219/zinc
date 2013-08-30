#include <cassert>
#include <zinc_plugin>

using namespace std;
using namespace pnp;

namespace HelloWorldPlugin
{
  void sayhello(Context* ctx, str_t remainder) {
    ctx->vtable->reply(ctx, "Hello, World!");
  }

  void install(PluginHost* host) {
    host->vtable->register_command(host, "helloworld", &sayhello);
  }

  static str_t plugin_name = "HelloWorldPlugin";
  static SemanticVersion plugin_version = { 0, 2, 0 };

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
  return &HelloWorldPlugin::plugbase;
}

pnp_module_t pnp_module =
{
  CUR_ZINC_VERSION,             // req_zinc_version
  nullptr,                      // init
  nullptr,                      // destroy

  1,                            // num_exported_plugins
  get_plugin                    // get_plugin
};
