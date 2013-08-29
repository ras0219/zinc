#include <cassert>
#include <zinc_plugin>

using namespace std;

namespace HelloWorldPlugin
{
  static void sayhello(Context* ctx, str_t remainder) {
    ctx->vtable->reply(ctx, "Hello, World!");
  }

  static str_t plugin_name = "HelloWorldPlugin";
  static SemanticVersion plugin_version = { 0, 2, 0 };
  static void install(PluginHostProxy* proxy) {
    proxy->register_command_handler("helloworld", &sayhello);
  }

  static PluginBase plugbase = {
    &install,
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
  nullptr,                      // init
  1,                            // num_exported_plugins
  get_plugin,                   // get_plugin
  nullptr                       // destroy
};
