#include "zinc_plugin_host"
#include <stdexcept>
#include <memory>

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
      throw std::runtime_error{"Failed to close dynamic library."};
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

  ZincPluginHost::~ZincPluginHost() {
    for (auto x : mods) {
      if (x.second.second != nullptr && x.second.second->destroy != nullptr)
        x.second.second->destroy();
      delete x.second.first;
    }
  }

  DynamicLib& ZincPluginHost::load_dl(const std::string& filename) {
    auto it = mods.find(filename);
    if (it == mods.end()) {
      auto dl = std::unique_ptr<DynamicLib>{new DynamicLib(filename, RTLD_NOW | RTLD_LOCAL)};
      pnp_module_t* pmt = dl->open_sym<pnp_module_t*>("pnp_module");
      if (pmt != nullptr && pmt->init != nullptr)
        pmt->init();
      it = mods.insert({ filename, { dl.release(), pmt } }).first;
    }
    return *it->second.first;
  }

  pnp_module_t* ZincPluginHost::load_plugin(const std::string& filename) {
    auto it = mods.find(filename);
    if (it == mods.end()) {
      auto dl = std::unique_ptr<DynamicLib>{new DynamicLib(filename, RTLD_NOW | RTLD_LOCAL)};
      pnp_module_t* pmt = dl->open_sym<pnp_module_t*>("pnp_module");
      if (pmt != nullptr && pmt->init != nullptr)
        pmt->init();
      it = mods.insert({ filename, { dl.release(), pmt } }).first;
    }
    return it->second.second;
  }

  void ZincPluginHost::unload_dl(const std::string& filename) {
    auto it = mods.find(filename);
    if (it != mods.end()) {
      if (it->second.second != nullptr && it->second.second->destroy != nullptr)
        it->second.second->destroy();
      delete it->second.first;
      mods.erase(it);
    }
  }

}
