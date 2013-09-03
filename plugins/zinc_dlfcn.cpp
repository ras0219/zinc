extern "C" {
#include <dlfcn.h>
}
#include <stdexcept>
#include <zinc_dlfcn>

using namespace zinc;

LibraryHandle::LibraryHandle(const char* fname, int flags)
  : filename(fname), hndl(dlopen(fname, flags))
{
  if (hndl == nullptr)
    throw std::runtime_error{dlerror()};

  * (void**)(&pmt) = dlsym(hndl, "pnp_module");

  if (pmt == nullptr)
    throw std::runtime_error{"invalid module"};
}
LibraryHandle::LibraryHandle(const std::string& fname, int flags)
  : filename(fname), hndl(dlopen(fname.c_str(), flags))
{
  if (hndl == nullptr)
    throw std::runtime_error{dlerror()};

  * (void**)(&pmt) = dlsym(hndl, "pnp_module");

  if (pmt == nullptr)
    throw std::runtime_error{"invalid module"};
}

LibraryHandle::~LibraryHandle()
{
  dlclose(hndl);
}
