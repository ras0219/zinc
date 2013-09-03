#include <cassert>
#include <zinc_plugin>
#include <zinc_io>
#include <zinc_interactive>

using namespace std;
using namespace zinc;

namespace HelloWorldPlugin
{
  void sayhello(OStream::Interface* os, str_t) {
    os->vtable->reply(os, "Hello, World!");
  }

  struct InstallInterfaces {
    Interactive::Interface* i;
  };

  void install(Interface** argv) {
    InstallInterfaces* ii = (InstallInterfaces*)(argv);
    ii->i->vtable->register_command(ii->i, "helloworld", &sayhello);
  }

  static str_t plugin_name = "HelloWorldPlugin";
  static SemanticVersion plugin_version = { 0, 3, 0 };
  
  InterfaceRequest requests[] = {
    Interactive::Request::required
  };

  static Plugin helloworldplugin = {
    &install,
    nullptr,
    nullptr,
    plugin_name,
    plugin_version,

    1,
    requests
  };

}

Plugin* get_plugin(std::size_t n) {
  assert(n == 0);
  return &HelloWorldPlugin::helloworldplugin;
}

pnp_module_t pnp_module =
{
  CUR_ZINC_VERSION,             // req_zinc_version
  1,                            // num_exported_plugins
  get_plugin                    // get_plugin
};
