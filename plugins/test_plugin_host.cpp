#include "zinc_plugin_host"
#include <iostream>
#include <iomanip>
#include <cassert>

using namespace std;
using namespace pnp;

struct DebugHostProxy {
  static PluginHostProxy proxy;

  static int register_command_handler(str_t base, command_cb ptr) {
    cout << "Call to register_command_handler. [base=";
    if (base == nullptr)
      cout << "nullptr";
    else
      cout << '"' << base << '"';

    cout << ", ";
    cout << hex << (unsigned long long int)ptr << dec << "]" << endl;

    return 0;
  };
};

PluginHostProxy DebugHostProxy::proxy = {
  &DebugHostProxy::register_command_handler
};

int main(int argc, char** argv) {
  if (argc == 1) {
    cout << "Usage: " << argv[0] << " <plugin>" << endl;
    return 0;
  }

  ZincPluginHost zph;

  pnp_module_t* mod = zph.load_plugin(string(argv[1]));

  assert(mod != nullptr);
  assert(mod->get_plugin != nullptr);

  cout << "Loaded dynamic library: " << argv[1] << endl;
  cout << "Number of plugins: " << mod->num_exported_plugins << endl << endl;
  cout << "Plugins:" << endl
       << "--------" << endl;

  str_t default_name = "<ERROR:NO_PLUGIN_NAME>";
  for (size_t x = 0; x < mod->num_exported_plugins; ++x) {
    PluginBase* pb = mod->get_plugin(x);
    if (pb->plugin_name == nullptr) {
      cout << "Error: Plugin field 'plugin_name' is 0." << endl;
      pb->plugin_name = default_name;
    }

    cout << setw(3) << x << " | " << setw(40) << pb->plugin_name << " | "
         << setw(6) << pb->plugin_version << endl;

    if (pb->plugin_name == default_name)
      pb->plugin_name = nullptr;
  }

  cout << endl << "Loading plugins..." << endl;
  for (size_t x = 0; x < mod->num_exported_plugins; ++x) {
    PluginBase* pb = mod->get_plugin(x);
    if (pb->install == nullptr) {
      cout << "Error: Plugin " << x << " does not have an install method." << endl;
    } else {
      cout << "Plugin" << x << "->install" << endl;
      pb->install(&DebugHostProxy::proxy);
    }
  }

  return 0;
}
