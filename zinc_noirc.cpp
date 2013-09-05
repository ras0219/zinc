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
#include <cctype>
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
#include <zinc_interactive>
#include <zinc_io>
#include "zinc_config.hpp"

struct zmq_pollitem_adapter {
  int fd;
  short events;
  short revents;
  operator zmq::pollitem_t() { return {0, fd, events, revents}; }
};

using namespace std;
using zinc::Interactive;
using zinc::OStream;

using string_t = const char*;

struct Bot {
  Bot()
    : interactive(Interactive::Impl<Bot, offsetof(Bot, interactive)>::interface)
    { }

  // This function handles messages that were directed at me
  void handle_message(string_t m) {
    if (m[0] == '-')
      return handle_command(m + 1);

    // Deal with other stuff here, like responding with funny quibbits
    string m2 = m;
    random_shuffle(m2.begin(), m2.end());
    cout <<  m2.c_str() << endl;
  }

  void handle_command(string_t m) {
    const char* eoc = strchr(m, ' ');
    string cmd;
    if (eoc == NULL)
      cmd.assign(m);
    else
      cmd.assign(m, eoc++);

    // Lowercase all commands.
    std::transform<string::iterator, string::iterator, int(int)>
      (cmd.begin(), cmd.end(), cmd.begin(), std::tolower);

    auto it = commands.find(cmd);
    if (it == commands.end()) {
      cerr << "Invalid command." << endl;
      return;
    }
    struct IRCReply {
      IRCReply()
        : os(OStream::Impl<IRCReply, offsetof(IRCReply, os)>::interface)
        { }

      void send(str_t msg) {
        cout << msg << endl;
      }

      OStream::Interface os;
    } reply;

    it->second(&reply.os, eoc);
  }

  // Functions required to implement the "Interactive" interface
  int register_command(str_t cmd, zinc::command_cb cb) {
    cerr << "Registered command '" << cmd << "'" << endl;
    auto it = commands.find(cmd);
    if (it != commands.end()) {
      cerr << "Registration failed." << endl;
      return -1;
    }
    commands.emplace(cmd, cb);
    return 0;
  }
  int unregister_command(str_t cmd) {
    cerr << "Unregistered command '" << cmd << "'" << endl;
    auto it = commands.find(cmd);
    if (it == commands.end()) {
      cerr << "Unregistration failed." << endl;
      return -1;
    }
    commands.erase(it);
    return 0;
  }

  Interactive::Interface interactive;
  map<string, zinc::command_cb> commands;
};

int main(int argc, const char** argv) {
  zmq::context_t context(1);

  zinc::Configuration config;

  if (argc > 1) {
    config.parse_file(argv[1]);
  } else {
    std::cout << "Warning! You have chosen to run zinc without any config file!\n"
              << "This means no plugins will be loaded "
              << "and we'll have next to no functionality! :(\n"
              << "\n"
              << "Please create a configuration json file and relaunch as\n"
              << "    " << argv[0] << " <configfile>\n" << std::endl;
  }

  Bot bot;

  zinc::DefaultKernel kern;

  kern.register_interface((zinc::Interface*)&bot.interactive);

  JSONValue* loadlibs = config.get_path(L"loadplugins");

  if (loadlibs != nullptr and loadlibs->IsObject()) {
    for (auto p : loadlibs->AsObject()) {
      string lib = zinc::get_locale_string(p.first);
      kern.load_library_file(lib.c_str());
      if (p.second->IsString()) {
        string plug = zinc::get_locale_string(p.second->AsString());
        if (kern.load_plugin(lib.c_str(), plug.c_str()) != 0)
          cerr << "Could not load '" << plug << "' from '" << lib << "'." << endl;
      }
      if (p.second->IsArray()) {
        for (auto q : p.second->AsArray()) {
          if (q->IsString()) {
            string plug = zinc::get_locale_string(q->AsString());
            if (kern.load_plugin(lib.c_str(), plug.c_str()) != 0)
              cerr << "Could not load '" << plug << "' from '" << lib << "'." << endl;
          }
        }
      }
    }
  }

  string line;

  while (getline(cin, line)) {
    bot.handle_message(line.c_str());
  }
}
