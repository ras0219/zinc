#include <iostream>
#include <sstream>
#include <cassert>
#include <zinc_plugin>
#include <zinc_interactive>
#include <zinc_io>
#include <zinc_channelio>

using namespace std;
using namespace zinc;

namespace IrcCmdPlugin
{
  // void listlibs(OStream::Interface* ctx, str_t) {
  //   stringstream ss;
  //   ss << "Loaded libraries: ";

  //   size_t num_libs = host->vtable->num_libraries(host);
  //   const LibraryInstance* libs[num_libs];
  //   size_t n = host->vtable->list_libraries(host, num_libs, libs);

  //   for (size_t x = 0; x < n; ++x) {
  //     ss << host->vtable->get_library_name(host, libs[x]);
  //     if (x != n-1)
  //       ss << ", ";
  //   }
  //   ctx->vtable->reply(ctx, ss.str().c_str());
  // }

  // void listplugins(Context* ctx, str_t) {
  //   stringstream ss;
  //   ss << "Loaded plugins: ";

  //   size_t num_plugs = host->vtable->num_loaded_plugins(host);
  //   const PluginInstance* plugs[num_plugs];
  //   size_t n = host->vtable->list_loaded_plugins(host, num_plugs, plugs);

  //   for (size_t x = 0; x < n; ++x) {
  //     const PluginBase* pb = host->vtable->get_loaded_plugin_base(host, plugs[x]);
  //     ss << pb->plugin_name << "[" << pb->version << "]";
  //     if (x != n-1)
  //       ss << ", ";
  //   }
  //   ctx->vtable->reply(ctx, ss.str().c_str());
  // }

  struct InstallInterfaces {
    Interactive::Interface* i;
    ChannelIO::Interface* ch;
  };

  InstallInterfaces* ii;

  void joinchannel(OStream::Interface* ctx, str_t remainder) {
    if (remainder == nullptr or *remainder == 0) {
      ctx->vtable->send(ctx, "Usage: -join <channel>");
      return;
    }
    ii->ch->vtable->join_channel(ii->ch, remainder);
  }

  void install(Interface** ifaces) {
    ii = (InstallInterfaces*)(ifaces);
    ii->i->vtable->register_command(ii->i, "join", &joinchannel);
    // ii->i->vtable->register_command(ii->i, "listlibs", &listlibs);
    // ii->i->vtable->register_command(ii->i, "listplugins", &listplugins);
  }
  void uninstall() {
    ii->i->vtable->unregister_command(ii->i, "join");
    // host->vtable->unregister_command(host, "listlibs");
    // host->vtable->unregister_command(host, "listplugins");
  }

  static str_t plugin_name = "IrcCmd";
  static SemanticVersion plugin_version = { 0, 2, 0 };

  InterfaceRequest requests[] = {
    Interactive::Request::required,
    ChannelIO::Request::required
  };

  static Plugin irccmdplugin = {
    &install,
    &uninstall, // uninstall
    nullptr, // transfer
    plugin_name,
    plugin_version,

    2,
    requests
  };
};

Plugin* get_plugin(std::size_t n) {
  assert(n == 0);
  return &IrcCmdPlugin::irccmdplugin;
}

pnp_module_t pnp_module =
{
  CUR_ZINC_VERSION,             // req_zinc_version
  1,                            // num_exported_plugins
  get_plugin                    // get_plugin
};
