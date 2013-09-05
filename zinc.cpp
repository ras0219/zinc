#include "ircpp.hpp"

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
#include <zinc_channelio>
#include <zinc_io>
#include <zinc_pollhost>
#include "zinc_config.hpp"
#include "zinc_zmq_pollhost.hpp"

using namespace std;
using zinc::Interactive;
using zinc::OStream;
using zinc::PollHost;
using zinc::ChannelIO;

struct Bot : public IRCSession {
  Bot(char const* servername,
      unsigned int port,
      char const* password,
      char const* nick_,
      char const* username,
      char const* realname)
    : IRCSession(servername, port, password, nick_, username, realname),
      interactive(Interactive::Impl<Bot, offsetof(Bot, interactive)>::interface),
      channelio(ChannelIO::Impl<Bot, offsetof(Bot, channelio)>::interface),
      nick(nick_)
    { }

  virtual void on_connect() {
    cout << "Connected." << endl;
  }
  virtual void on_nick(string_t origin, string_t nick) { }
  virtual void on_quit(string_t origin, string_t reason) { }
  virtual void on_join(string_t origin, string_t channel) { }
  virtual void on_part(string_t origin, string_t channel, string_t reason) { }
  virtual void on_numeric(unsigned int, char const *, char const **, unsigned int) { }
  virtual void on_channel(string_t origin, string_t channel, string_t m) {
    if (m == nullptr || m[0] == 0)
      return;

    if (m[0] == '-')
      return handle_command(origin, channel, m + 1);

    if (strncmp(m, nick.c_str(), nick.size()) == 0
        && m[nick.size()] == ':'
        && m[nick.size() + 1] == ' ')
      return handle_message(origin, channel, m + nick.size() + 2);
  }
  virtual void on_privmsg(string_t origin, string_t yourname, string_t m) {
    if (m == nullptr || m[0] == 0)
      return;

    return handle_message(origin, origin, m);
  }

  // This function handles messages that were directed at me
  void handle_message(string_t origin, string_t channel, string_t m) {
    if (m[0] == '-')
      return handle_command(origin, channel, m + 1);

    // Deal with other stuff here, like responding with funny quibbits
    string m2 = m;
    random_shuffle(m2.begin(), m2.end());
    msg(channel, m2.c_str());
  }

  void handle_command(string_t origin, string_t channel, string_t m) {
    const char* eoc = strchr(m, ' ');
    string cmd;
    if (eoc == NULL)
      cmd.assign(m);
    else
      cmd.assign(m, eoc++);

    // Lowercase all commands.
    std::transform<string::iterator, string::iterator, int(int)>
      (cmd.begin(), cmd.end(), cmd.begin(), std::tolower);

    // Do handling logic here
    if (cmd == "source")
      return msg(channel, "https://github.com/ras0219/zinc");

    auto it = commands.find(cmd);
    if (it == commands.end()) {
      notice(origin, "Invalid command");
      return;
    }
    struct IRCReply {
      IRCReply(str_t chan, Bot& b)
        : os(OStream::Impl<IRCReply, offsetof(IRCReply, os)>::interface),
          ch(chan),
          bot(b)
        { }

      void send(str_t msg) {
        bot.msg(ch, msg);
      }

      OStream::Interface os;
      string_t ch;
      Bot& bot;
    } reply(channel, *this);

    it->second(&reply.os, eoc);
  }

  // Functions required to implement the "ChannelIO" interface
  void join_channel(str_t channel) {
    join(channel);
  }

  void send_message(str_t channel, str_t m) {
    msg(channel, m);
  }

  // Functions required to implement the "Interactive" interface
  int register_command(str_t cmd, zinc::command_cb cb) {
    auto it = commands.find(cmd);
    if (it != commands.end())
      return -1;
    commands.emplace(cmd, cb);
    return 0;
  }
  int unregister_command(str_t cmd) {
    auto it = commands.find(cmd);
    if (it == commands.end())
      return -1;
    commands.erase(it);
    return 0;
  }
  std::size_t num_avail_commands() { return commands.size(); }
  void get_avail_commands(str_t* cmds) {
    for (auto p : commands)
      *cmds++ = p.first.c_str();
  }

  Interactive::Interface interactive;
  ChannelIO::Interface channelio;

  map<string, zinc::command_cb> commands;
  string nick;
};

struct PollCB {
  static Bot* bot;
    
  static void poll_cb(zmq::pollitem_t fd) {
    bot->process_poll_descriptors<ZMQ_POLLIN, ZMQ_POLLOUT>(&fd, &fd + 1);
  }
};
Bot* PollCB::bot = nullptr;

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

  //MegaHalService mhserv(context, global_config.get_string_config(L"brainsocket").c_str());

  string
    serverpass = config.get_string_config(L"serverpass", ""),
    serveraddr = config.get_string_config(L"serveraddr", "irc.freenode.net"),
    username = config.get_string_config(L"username", "cxxtestbot"),
    nickname = config.get_string_config(L"nickname", "cxxtestbot"),
    realname = config.get_string_config(L"realname", "C++11 Test Bot");
  int port = config.get_int_config(L"serverport", 6667);

  Bot s(serveraddr.c_str(),
        port,
        serverpass.empty() ? NULL : serverpass.c_str(),
        username.c_str(),
        nickname.c_str(),
        realname.c_str());

  zinc::ZMQ_PollHost zmqph;
  {
    PollCB::bot = &s;

    s.add_poll_descriptors<zinc::zmq_pollitem_adapter, ZMQ_POLLIN, ZMQ_POLLOUT>
      (std::back_inserter(zmqph.pollfds));
    auto sz = zmqph.pollfds.size();
    for (size_t x = 0; x < sz; ++x)
      zmqph.pollcbs.push_back(PollCB::poll_cb);
  }

  zinc::DefaultKernel kern;

  kern.register_interface((zinc::Interface*)&s.interactive);
  kern.register_interface((zinc::Interface*)&s.channelio);
  kern.register_interface((zinc::Interface*)&zmqph.pollhost);

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

  while (true) {
    zmqph.poll_once();
  }
}
