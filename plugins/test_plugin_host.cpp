#include <zinc_plugin_host>
#include <zinc_interactive>
#include <iostream>
#include <cstring>
#include <iomanip>
#include <cassert>

using namespace std;
using namespace zinc;

struct DebugInteractive {
  int register_command(str_t cmd, command_cb cb) {
    cout << "Call to register_command. [base=";
    if (cmd == nullptr)
      cout << "nullptr";
    else
      cout << '"' << cmd << '"';

    cout << ", ";
    cout << hex << (size_t)cb << dec << "]" << endl;

    return 0;
  }

  int unregister_command(str_t cmd) {
    cout << "Call to unregister_command. [base=";
    if (cmd == nullptr)
      cout << "nullptr";
    else
      cout << '"' << cmd << '"';

    cout << "]" << endl;

    return 0;
  }

  DebugInteractive()
    : interactive(Interactive
                  ::Impl<DebugInteractive, offsetof(DebugInteractive, interactive)>
                  ::interface)
    {}

  Interactive::Interface interactive;
};

int main(int argc, char** argv) {
  if (argc < 2) {
    cout << "Usage: " << argv[0] << " <library> [<plugin>]" << endl;
    return 0;
  }

  LibraryManager libman;

  LibraryHandle* lib = libman.load_library_file(argv[1]);

  pnp_module_t* mod = lib->pmt;

  assert(mod != nullptr);
  assert(mod->get_plugin != nullptr);

  cout << "Loaded dynamic library: " << argv[1] << endl;
  cout << "Required Zinc Version: " << mod->req_zinc_version << endl;
  assert(compatible(mod->req_zinc_version, CUR_ZINC_VERSION));
  cout << "Number of plugins: " << mod->num_exported_plugins << endl << endl;
  cout << "Plugins:" << endl
       << "--------" << endl;

  str_t default_name = "<ERROR:NO_PLUGIN_NAME>";
  for (size_t x = 0; x < mod->num_exported_plugins; ++x) {
    Plugin* pb = mod->get_plugin(x);
    if (pb->name == nullptr) {
      cout << "Error: Plugin field 'name' is null." << endl;
      pb->name = default_name;
    }

    cout << setw(3) << x << " | " << setw(40) << pb->name << " | "
         << setw(6) << pb->version << endl;

    if (pb->name == default_name)
      pb->name = nullptr;
  }

  if (argc < 3)
    return 0;


  Plugin* plug = nullptr;
  for (size_t x = 0; x < mod->num_exported_plugins; ++x) {
    Plugin* pb = mod->get_plugin(x);
    if (strcmp(pb->name, argv[2]) == 0) {
      plug = pb;
      break;
    }
  }
  if (plug == nullptr) {
    cout << endl << "Could not locate plugin '" << argv[2] << "'" << endl;
    return -1;
  }

  InterfaceUnifier unifier;
  DebugInteractive di;
  unifier.available.push_back((Interface*)&di.interactive);

  cout << endl << "Loading plugin '" << plug->name << "'" << endl;
  cout << "Requirements: " << plug->num_reqs << endl;
  for (size_t x = 0; x < plug->num_reqs; ++x)
    cout << setw(3) << x << " | " << plug->reqs[x] << endl;

  vector<Interface*> ifaces = unifier.unify_request(plug);
  cout << ifaces.size() << endl;
  if (ifaces.size() != plug->num_reqs) {
    cout << endl << "Could not satisfy requirements of plugin." << endl;
    return 0;
  }

  cout << "Installing..." << endl;
  assert(plug->install);
  plug->install(ifaces.data());
  cout << "Uninstalling...." << endl;
  assert(plug->uninstall);
  plug->uninstall();

  return 0;
}
