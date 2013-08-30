#include "zinc_plugin_host"
#include <iostream>
#include <stdexcept>
#include <cassert>
#include <memory>

using namespace std;

namespace pnp {

  Handle open_dl(const char* filename, int flags) {
    Handle h = dlopen(filename, flags);
    if (h == NULL)
      throw std::runtime_error{dlerror()};
    return h;
  }

  void* open_sym_internal(Handle h, const char* sym) {
    void* lsym = dlsym(h, sym);

    const char* errmsg = dlerror();
    if (errmsg)
      throw std::runtime_error{errmsg};

    return lsym;
  }

  void close_dl(Handle h) {
    int err = dlclose(h);
    if (err)
      throw std::runtime_error{"failed to close dynamic library"};
  }

  DynamicLib::DynamicLib(const char* fname, int flags)
    : hndl{open_dl(fname, flags)} {}
  DynamicLib::DynamicLib(const std::string& fname, int flags)
    : hndl{open_dl(fname.c_str(), flags)} {}
  DynamicLib::~DynamicLib() {
    // Cannot simply call close_dl, because that could throw an exception (really bad!)
    dlclose(hndl);
    // TODO: add some kind of logging if an error occurs here
  }

  // LibraryInstance
  LibraryInstance::LibraryInstance(const std::string& fname)
    : hndl(fname, RTLD_NOW | RTLD_LOCAL), filename(fname)
  {
    module_info = hndl.open_sym<pnp_module_t*>("pnp_module");
    if (module_info == nullptr)
      throw std::runtime_error{fname + " is not a valid plugin library"};
    // Need to check required zinc version here
    if (not compatible(CUR_ZINC_VERSION, module_info->req_zinc_version))
      throw std::runtime_error{fname + " is not compatible with this zinc version"};

    if (module_info->init != nullptr)
      module_info->init();
  }

  LibraryInstance::~LibraryInstance() {
    // Unloading libraries not currently supported.
    assert(false);
  }

  ZincPluginHost::~ZincPluginHost() {
    // Unloading libraries not currently supported.
    assert(false);
    // for (auto x : mods) {
    //   if (x.second.second != nullptr && x.second.second->destroy != nullptr)
    //     x.second.second->destroy();
    //   delete x.second.first;
    // }
  }

  void ZincPluginHost::add_library_file(const std::string& filename) {
    std::cerr << "add_library_file(" << filename << ")" << std::endl;
    if (filename_to_linst.find(filename) != filename_to_linst.end()) {
      std::cerr << "Plugin Library \"" << filename << "\" is already loaded!" << std::endl;
      return;
    }
    auto libptr = new LibraryInstance(filename);
    assert(libptr);
    auto lib = std::unique_ptr<LibraryInstance>{libptr};
    filename_to_linst.insert({filename, lib.get()});
    lib.release();
  }

  size_t ZincPluginHost::num_libraries() { return filename_to_linst.size(); }
  ZincPluginHost::lib_iterator ZincPluginHost::libraries_begin() {
    return filename_to_linst.begin();
  }
  ZincPluginHost::lib_iterator ZincPluginHost::libraries_end() {
    return filename_to_linst.end();
  }

  size_t ZincPluginHost::num_plugins() { return linst_to_plugins.size(); }
  ZincPluginHost::plugin_iterator ZincPluginHost::plugins_begin() {
    return linst_to_plugins.begin();
  }
  ZincPluginHost::plugin_iterator ZincPluginHost::plugins_end() {
    return linst_to_plugins.end();
  }

  size_t ZincPluginHost::num_plugins(LibraryInstance::sptr p) {
    return linst_to_plugins.count(p);
  }
  ZincPluginHost::plugin_iterator ZincPluginHost::plugins_begin(LibraryInstance::sptr p) {
    return linst_to_plugins.lower_bound(p);
  }
  ZincPluginHost::plugin_iterator ZincPluginHost::plugins_end(LibraryInstance::sptr p) {
    return linst_to_plugins.upper_bound(p);
  }

  PluginBase* ZincPluginHost::load_plugin(const string& filename,
                                          const string& plugin_name,
                                          PluginHost* ph) {
    LibraryInstance::sptr lib = filename_to_linst[filename];
    if (lib == nullptr)
      throw std::runtime_error{filename + " is not loaded"};
    assert(lib);
    size_t plug_num = lib->module_info->num_exported_plugins;
    for (size_t i = 0; i < plug_num; ++i) {
      PluginBase* pb = lib->module_info->get_plugin(i);
      assert(pb);
      if (pb->plugin_name == plugin_name) {
        // Found the plugin
        auto it = plugins_begin(lib);
        while (it != plugins_end(lib))
          if (it->second == pb)
            // Plugin is already loaded...
            return nullptr;
        // Great, plugin isn't loaded yet!

        // TODO: use a wrapper to ensure pb is uninstalled if insert fails
        pb->install(ph);
        linst_to_plugins.insert({lib, pb});
        return pb;
      }
      ++plug_num;
    }

    // Did not find the plugin
    return nullptr;
  }
  // DynamicLib& ZincPluginHost::load_dl(const std::string& filename) {
  //   auto it = mods.find(filename);
  //   if (it == mods.end()) {
  //     auto dl = std::unique_ptr<DynamicLib>{new DynamicLib(filename, RTLD_NOW | RTLD_LOCAL)};
  //     pnp_module_t* pmt = dl->open_sym<pnp_module_t*>("pnp_module");
  //     if (pmt != nullptr && pmt->init != nullptr)
  //       pmt->init();
  //     it = mods.insert({ filename, { dl.release(), pmt } }).first;
  //   }
  //   return *it->second.first;
  // }

  // pnp_module_t* ZincPluginHost::load_plugin(const std::string& filename) {
  //   auto it = mods.find(filename);
  //   if (it == mods.end()) {
  //     auto dl = std::unique_ptr<DynamicLib>{new DynamicLib(filename, RTLD_NOW | RTLD_LOCAL)};
  //     pnp_module_t* pmt = dl->open_sym<pnp_module_t*>("pnp_module");
  //     if (pmt != nullptr && pmt->init != nullptr)
  //       pmt->init();
  //     it = mods.insert({ filename, { dl.release(), pmt } }).first;
  //   }
  //   return it->second.second;
  // }

  // void ZincPluginHost::unload_dl(const std::string& filename) {
  //   auto it = mods.find(filename);
  //   if (it != mods.end()) {
  //     if (it->second.second != nullptr && it->second.second->destroy != nullptr)
  //       it->second.second->destroy();
  //     delete it->second.first;
  //     mods.erase(it);
  //   }
  // }

}
