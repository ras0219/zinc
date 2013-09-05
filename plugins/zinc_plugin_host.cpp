#include <iostream>
#include <stdexcept>
#include <cstring>
#include <cassert>
#include <zinc_plugin_host>

using namespace std;
using namespace zinc;

LibraryHandle* LibraryManager::load_library_file(const std::string& filename) {
  auto it = libraries.find(filename);
  if (it != libraries.end()) {
    std::cerr << "Plugin Library \"" << filename << "\" is already loaded!" << std::endl;
    return it->second;
  }
  auto lib = new LibraryHandle(filename);
  // TODO: ensure library is freed if oom
  libraries.insert({filename, lib});
  return lib;
}

Plugin* LibraryManager::locate_plugin(const string& filename,
                                      const string& pluginname)
{
  LibraryHandle* lib = libraries[filename];
  if (lib == nullptr)
    throw std::runtime_error{filename + " is not loaded"};

  const size_t plug_num = lib->pmt->num_exported_plugins;
  for (size_t i = 0; i < plug_num; ++i) {
    Plugin* pb = lib->pmt->get_plugin(i);

    assert(pb);
    if (pb->name == pluginname)
      return pb;
  }

  // Did not find the plugin
  return nullptr;
}

Interface*
InterfaceUnifier::unify_request(const InterfaceRequest& req)
{
  for (auto i : available) {
    cout << *i << endl;
    cout << strcmp(i->ip->name, req.name) << " && "
         << compatible(i->ip->version, req.version)
         << endl;
    if (strcmp(i->ip->name, req.name) == 0 && compatible(i->ip->version, req.version))
      return i;
  }

  return nullptr;
}

std::vector<Interface*>
InterfaceUnifier::unify_request(std::size_t nreqs, const InterfaceRequest* reqs)
{
  std::vector<Interface*> ifaces;
  ifaces.reserve(nreqs);
  for (size_t x = 0; x < nreqs; ++x) {
    auto res = unify_request(reqs[x]);
    if (res == nullptr && reqs[x].required) {
      cout << "Failed to provide required interface: " << reqs[x] << endl;
      ifaces.clear();
      break;
    }
    cout << "Providing interface: " << *res << endl;
    ifaces.push_back(res);
  }
  return ifaces;
}
