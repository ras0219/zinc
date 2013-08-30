#include <iostream>
#include <sstream>
#include <cassert>
#include <zinc_plugin>

using namespace std;
using namespace pnp;

namespace IrcCmdPlugin
{
  static PluginHost* host = nullptr;

  void joinchannel(Context* ctx, str_t remainder) {
    if (remainder == nullptr or *remainder == 0) {
      ctx->vtable->reply(ctx, "Usage: -join <channel>");
      return;
    }
    host->vtable->irc_join(host, remainder);
  }

  void listlibs(Context* ctx, str_t) {
    stringstream ss;
    ss << "Loaded libraries: ";

    size_t num_libs = host->vtable->num_libraries(host);
    const LibraryInstance* libs[num_libs];
    size_t n = host->vtable->list_libraries(host, num_libs, libs);

    for (size_t x = 0; x < n; ++x) {
      ss << host->vtable->get_library_name(host, libs[x]);
      if (x != n-1)
        ss << ", ";
    }
    ctx->vtable->reply(ctx, ss.str().c_str());
  }

  void listplugins(Context* ctx, str_t) {
    stringstream ss;
    ss << "Loaded plugins: ";

    size_t num_plugs = host->vtable->num_loaded_plugins(host);
    const PluginInstance* plugs[num_plugs];
    size_t n = host->vtable->list_loaded_plugins(host, num_plugs, plugs);

    for (size_t x = 0; x < n; ++x) {
      const PluginBase* pb = host->vtable->get_loaded_plugin_base(host, plugs[x]);
      ss << pb->plugin_name << "[" << pb->version << "]";
      if (x != n-1)
        ss << ", ";
    }
    ctx->vtable->reply(ctx, ss.str().c_str());
  }

  void install(PluginHost* host_) {
    assert(host == nullptr);
    host = host_;
    assert(host);
    host->vtable->register_command(host, "join", &joinchannel);
    host->vtable->register_command(host, "listlibs", &listlibs);
    host->vtable->register_command(host, "listplugins", &listplugins);
  }
  void uninstall() {
    assert(host);
    host->vtable->unregister_command(host, "join");
    host->vtable->unregister_command(host, "listlibs");
    host->vtable->unregister_command(host, "listplugins");
    host = nullptr;
  }

  static str_t plugin_name = "IrcCmd";
  static SemanticVersion plugin_version = { 0, 1, 1 };

  static PluginBase plugbase = {
    &install,
    &uninstall, // uninstall
    nullptr, // transfer
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
  CUR_ZINC_VERSION,             // req_zinc_version
  nullptr,                      // init
  nullptr,                      // destroy

  1,                            // num_exported_plugins
  get_plugin                    // get_plugin
};
