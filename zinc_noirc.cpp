extern "C" {
#include <poll.h>
}

#include <cassert>
#include <cstring>
#include <cmath>
/*#include <type>*/
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <memory>
#include <vector>
#include <deque>
#include <map>
#include <set>

// #include "boost/filesystem.hpp"
#include "zmq.hpp"
#include "JSON.h"
#include "zmqservice.hpp"
#include <zinc_plugin_host>

using namespace std;

// from StackOverflow
// http://stackoverflow.com/questions/2602013/read-whole-ascii-file-into-c-stdstring
// Modified for unicode
wstring get_file_contents(const char *filename)
{
  wifstream in(filename, ios::in | ios::binary);
  if (in)
  {
    wstring contents;
    in.seekg(0, ios::end);
    contents.resize(in.tellg());
    in.seekg(0, ios::beg);
    in.read(&contents[0], contents.size());
    in.close();
    return(contents);
  }
  throw(errno);
}

// Dummy
std::string get_locale_string(const std::string & s)
{
  return s;
}

// Real worker
std::string get_locale_string(const std::wstring & s)
{
  const wchar_t * cs = s.c_str();
  const size_t wn = std::wcsrtombs(NULL, &cs, 0, NULL);

  if (wn == size_t(-1))
  {
    std::cout << "Error in wcsrtombs(): " << errno << std::endl;
    return "";
  }

  std::vector<char> buf(wn + 1);
  const size_t wn_again = std::wcsrtombs(buf.data(), &cs, wn + 1, NULL);

  if (wn_again == size_t(-1))
  {
    std::cout << "Error in wcsrtombs(): " << errno << std::endl;
    return "";
  }

  assert(cs == NULL); // successful conversion

  return std::string(buf.data(), wn);
}

struct Configuration {
  void parse_file(const char* filename) {
    wstring config = get_file_contents(filename);
    JSONValue *value = JSON::Parse(config.c_str());

    if (value == nullptr)
      throw runtime_error("Failed to parse config file");

    if (not value->IsObject())
      throw runtime_error("Invalid config file");

    JSONObject root = value->AsObject();

    for (auto p : root)
      if (p.second->IsString())
        string_configs.insert({p.first, p.second->AsString()});
      else if (p.second->IsNumber())
        num_configs.insert({p.first, (int)round(p.second->AsNumber())});
      else if (p.first == L"loadplugins" && p.second->IsObject()) {
        for (auto q : p.second->AsObject()) {
          libraries_to_load.insert(get_locale_string(q.first));
          if (q.second->IsString())
            plugs_to_load.emplace(get_locale_string(q.first),
                                  get_locale_string(q.second->AsString()));
          else if (q.second->IsArray())
            for (auto r : q.second->AsArray())
              if (r->IsString())
                plugs_to_load.emplace(get_locale_string(q.first),
                                      get_locale_string(r->AsString()));
        }
      }
  }

  inline string get_string_config(const wstring& key) {
    return get_locale_string(string_configs[key]);
  }
  inline int get_num_config(const wstring& key) {
    return num_configs[key];
  }

  map<wstring, wstring> string_configs;
  map<wstring, int> num_configs;

  set< string > libraries_to_load;
  set< pair<string, string> > plugs_to_load;
} global_config{
  { },
  { },
  { },
  { }
  };

struct IO_Concept {
  int register_command(str_t cmd, pnp::command_cb ptr) {
    // actual implementation here
    if (cmd_handlers.find(cmd) != cmd_handlers.end())
      return -1;
    cmd_handlers.insert({cmd, ptr});
    return 0;
  }

  int unregister_command(str_t cmd) {
    auto it = cmd_handlers.find(cmd);
    if (it == cmd_handlers.end())
      return -1;

    cmd_handlers.erase(it);
    return 0;
  }

  int register_fallback(pnp::fallback_cb ptr) {
    // actual implementation here
    fallback_handlers.push_back(ptr);
    return 0;
  }

  int unregister_fallback(pnp::fallback_cb ptr) {
    auto it = std::find(fallback_handlers.begin(),
                        fallback_handlers.end(),
                        ptr);
    if (it == fallback_handlers.end())
      return -1;

    fallback_handlers.erase(it);
    return 0;
  }

  void irc_join(str_t channel) {
    
  }

  void irc_msg(str_t target, str_t msg) {
    
  }

  map<string, pnp::command_cb> cmd_handlers;
  vector<pnp::fallback_cb> fallback_handlers;
} ioc;

int main(int argc, const char** argv) {
  zmq::context_t context(1);

  if (argc > 1) {
    global_config.parse_file(argv[1]);
  } else {
    std::cout << "Warning! You have chosen to run zinc without any config file!\n"
              << "This means no plugins will be loaded "
              << "and we'll have next to no functionality! :(\n"
              << "\n"
              << "Please create a configuration json file and relaunch as\n"
              << "    " << argv[0] << " <configfile>\n" << std::endl;
  }

  pnp::ZincPluginHost zph;

  struct ZincPluginHostProxy : pnp::PluginHostThunk<ZincPluginHostProxy> {
    ZincPluginHostProxy(pnp::ZincPluginHost& z) : zph(z) {}

    int register_command(str_t cmd, pnp::command_cb ptr) {
      return ioc.register_command(cmd, ptr);
    }

    int unregister_command(str_t cmd) {
      return ioc.unregister_command(cmd);
    }

    int register_fallback(pnp::fallback_cb ptr) {
      return ioc.register_fallback(ptr);
    }

    int unregister_fallback(pnp::fallback_cb ptr) {
      return ioc.unregister_fallback(ptr);
    }

    size_t num_libraries() { return zph.num_libraries(); }
    size_t list_libraries(size_t n, const pnp::LibraryInstance** libs) {
      size_t z = min(n, zph.num_libraries());
      auto it = zph.libraries_begin();
      for (size_t x = 0; x < z; ++x) {
        libs[x] = it->second;
        ++it;
      }
      return z;
    }
    str_t get_library_name(const pnp::LibraryInstance* lib) {
      return lib->filename.c_str();
    }

    size_t num_loaded_plugins() { return zph.num_plugins(); }
    size_t list_loaded_plugins(size_t n, const pnp::PluginInstance** plugs) {
      size_t z = min(n, zph.num_plugins());
      auto it = zph.plugins_begin();
      for (size_t x = 0; x < z; ++x) {
        plugs[x] = it->second;
        ++it;
      }
      return z;
    }
    const pnp::PluginBase* get_loaded_plugin_base(const pnp::PluginInstance* plug) {
      return plug;
    }

    size_t num_provided_plugins(const pnp::LibraryInstance* lib) {
      return lib->module_info->num_exported_plugins;
    }
    size_t list_provided_plugins(const pnp::LibraryInstance* lib,
                                 size_t n,
                                 const pnp::PluginBase** plugs) {
      assert(false);
      return 0;
    }

    void irc_join(str_t channel) {
      ioc.irc_join(channel);
    }

    void irc_msg(str_t target, str_t msg) {
      ioc.irc_msg(target, msg);
    }

    pnp::ZincPluginHost& zph;
  } zphp(zph);

  for (auto f : global_config.libraries_to_load) {
    cout << "Loading <" << f << ">..." << endl;
    zph.add_library_file(f);
  }

  for (auto p : global_config.plugs_to_load) {
    cout << "Loading <" << p.second << "> from <" << p.first << ">..." << endl;
    pnp::PluginBase* pb = zph.load_plugin(p.first, p.second, &zphp);
    if (pb == nullptr)
      cout << "Warning!! Could not load " << p.second << endl;
  }

  // Finished loading. Time to quit.
}
