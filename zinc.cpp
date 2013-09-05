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
#include <zinc_io>
#include "zinc_config.hpp"

// enum megahal_cmd { LEARN, REPLY };
// struct MessageT {
//   std::string msg;
//   megahal_cmd cmd;
//   std::function< void(const std::string&) > cb;
// };

// struct MegaHalService : ZMQService<MessageT> {
//   MegaHalService(zmq::context_t& ctx, const char* addr)
//     : ZMQService<MessageT>(ctx)
//     {
//       socket.connect(addr);
//     }

//   virtual zmq::message_t request(const MessageT& m) {
//     zmq::message_t msg(m.msg.size() + 3);
//     if (m.cmd == LEARN)
//       memcpy(static_cast<char*>(msg.data()), "LRN", 3);
//     else
//       memcpy(static_cast<char*>(msg.data()), "REP", 3);
//     memcpy(static_cast<char*>(msg.data()) + 3, m.msg.data(), m.msg.size());
//     return msg;
//   }
//   virtual void response(const MessageT& m, zmq::message_t& msg) {
//     std::string st((const char*)msg.data(), msg.size());
//     m.cb(st);
//   }
// };

// struct IRCChannelContext : pnp::ContextThunk<IRCChannelContext> {
//   IRCChannelContext(str_t channel, str_t user, IRCSession& mb)
//     : usr(user), chan(channel), irc(mb) {}
  
//   void reply(str_t msg) {
//     irc.msg(chan, msg);
//   }
//   bool is_irc() { return true; }
//   bool is_local() { return false; }
//   bool is_other() { return false; }

//   str_t irc_req_user() { return usr; }
//   str_t irc_req_channel() { return chan; }

//   str_t usr;
//   str_t chan;
//   IRCSession& irc;
// };


// struct MyBot : public IRCSession {
//   MyBot(char const* servername, unsigned int port, char const*
//         password, const char* nick_, const char* username, const char*
//         realname, ZMQService<MessageT>& srv)
//     : IRCSession{servername, port, password, nick_, username, realname},
//     nick{nick_},
//     privmsg_srv(srv)
//     { }

//   virtual void on_connect() {
//     std::cout << "MyBot Connected.\n";
//   }
//   virtual void on_nick(string_t origin, string_t nick) { }
//   virtual void on_quit(string_t origin, string_t reason) { }
//   virtual void on_join(string_t origin, string_t channel) {
//     if (origin == nick) {
//       // I have successfully joined a channel!
//       // msg(channel, "Fear not, I have arrived.");

//       // do... something?
//       std::cout << "Succesfully joined <" << channel << ">" << std::endl;
//     } else {
//       std::cout << origin << " succesfully joined <" << channel << ">" << std::endl;
//     }
//   }
//   virtual void on_part(string_t origin, string_t channel, string_t reason) { }
//   virtual void on_channel(string_t origin, string_t channel, string_t m) {
//     std::cout << "[" << channel << "] <" << origin << "> \""
//               << m << "\"" << std::endl;

//     if (m == nullptr || m[0] == 0)
//       // Ignore empty messages
//       return;

//     if (m[0] == '-') {
//       // Process a command
//       const char* eoc = strchr(m, ' ');

//       std::string cmd;
//       if (eoc == NULL)
//         cmd.assign(m + 1);
//       else
//         cmd.assign(m, eoc++);

//       // Lowercase all commands.
//       transform(cmd.begin(), cmd.end(), cmd.begin(), tolower);

//       handle_command(cmd, origin, channel, eoc);
//     }
//     else if (strncmp(m, nick.c_str(), nick.size()) == 0
//              && m[nick.size()] == ':'
//              && m[nick.size() + 1] == ' ')
//     {
//       // I'm hilightted! I'm special! Let's give them a special treat!
//       std::string newmsg = m + nick.size() + 2;
//       std::cout << "<" << channel << "/" << origin << "> " << newmsg << std::endl;

//       std::string channel_s = channel;
//       privmsg_srv.send({newmsg, REPLY, [this,channel_s](const std::string& reply) {
//             msg(channel_s.c_str(), reply.c_str());
//           }});
//     } else {
//       IRCChannelContext irccc(channel, origin, *this);
//       auto it = fallback_handlers.rbegin();
//       while (it != fallback_handlers.rend()) {
//         if (!(*it)(&irccc, m))
//           return;
//         ++it;
//       }

//       // Message wasn't a command. We should remove highlights before learning...
//       // On second thought that's hard.... forget about it
//       std::string st = m;
//       if (st[0] == '<') {
//         auto it = st.begin();
//         while (it != st.end() && *it != '>') ++it;
//         if (it != st.end()) ++it;
//         if (it != st.end()) ++it;
//         std::string new_msg = std::string(it, st.end());
//         privmsg_srv.send({new_msg, LEARN, [](const std::string& reply) {}});
//       } else {
//         privmsg_srv.send({st, LEARN, [](const std::string& reply) {}});
//       }
//     }
//   }
//   virtual void on_privmsg(string_t origin, string_t yourname, string_t m) {
//     try {
//       std::cout << "<" << origin << "> " << m << "\n";

//       if (m == nullptr || m[0] == 0)
//         // Ignore empty messages
//         return;

//       if (m[0] == '-') {
//         // Process a command
//         const char* eoc = strchr(m, ' ');

//         std::string cmd;
//         if (eoc == NULL)
//           cmd.assign(m + 1);
//         else
//           cmd.assign(m + 1, eoc++);

//         // Lowercase all commands.
//         transform(cmd.begin(), cmd.end(), cmd.begin(), tolower);

//         handle_command(cmd, origin, origin, eoc);
//       } else {
//         IRCChannelContext irccc(origin, origin, *this);
//         auto it = fallback_handlers.rbegin();
//         while (it != fallback_handlers.rend()) {
//           if (!(*it)(&irccc, m))
//             return;
//           ++it;
//         }

//         std::string origin_s = origin;
//         privmsg_srv.send({m, REPLY, [this,origin_s](const std::string& reply) {
//               msg(origin_s.c_str(), reply.c_str());
//             }});
//       }
//     } catch (std::exception& e) {
//       msg(origin, e.what());
//     }
//   }

//   virtual void on_numeric(unsigned int event, char const * origin, char const
//                           ** params, unsigned int count) {
//     std::cout << "MyBot Received Numeric Message.\n";
//     for (unsigned int n = 0; n < count; ++n) {
//       std::cout << params[n] << "\n";
//     }
//   }

//   void handle_command(const std::string& cmd, string_t origin,
//                       string_t channel, string_t rem) {
//     if (cmd == "source") {
//       msg(channel, "https://github.com/ras0219/zinc");
//       return;
//     }
//     auto it = cmd_handlers.find(cmd);
//     if (it != cmd_handlers.end()) {
//       IRCChannelContext irccc(channel, origin, *this);
      
//       it->second(&irccc, rem);
//     }
//   }

//   std::map<std::string, pnp::command_cb> cmd_handlers;
//   std::vector<pnp::fallback_cb> fallback_handlers;

// private:
//   std::string nick;
//   ZMQService<MessageT>& privmsg_srv;
// };

struct zmq_pollitem_adapter {
  int fd;
  short events;
  short revents;
  operator zmq::pollitem_t() { return {0, fd, events, revents}; }
};

// struct Configuration {
// } global_config{
//   {
//     {L"brainsocket", L"tcp://localhost:5555"},
//     {L"serveraddr", L"irc.freenode.net"},
//     {L"serverpass", L""},
//     {L"username", L"cxxtestbot"},
//     {L"nickname", L"cxxtestbot"},
//     {L"realname", L"C++11 Test Bot"}
//   },
//   {
//     {L"serverport", 6667}
//   },
//   {},
//   {}
//   };

using namespace std;
using zinc::Interactive;
using zinc::OStream;

struct Bot : public IRCSession {
  Bot(char const* servername,
      unsigned int port,
      char const* password,
      char const* nick_,
      char const* username,
      char const* realname)
    : IRCSession(servername, port, password, nick_, username, realname),
      interactive(Interactive::Impl<Bot, offsetof(Bot, interactive)>::interface),
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

  Interactive::Interface interactive;

  map<string, zinc::command_cb> commands;
  string nick;
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
          realname.c_str()// ,
          // mhserv
    );

  zinc::DefaultKernel kern;

  kern.register_interface((zinc::Interface*)&s.interactive);

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

  std::vector<zmq::pollitem_t> pollfds;

  while (true) {
    pollfds.clear();

    // Add the zmq poll descriptors to the beginning
    //mhserv.add_to_zmq_pollfds(std::back_inserter(pollfds));

    // Mark where irc's pollfds begin
    auto irc_begin_nth = pollfds.size();

    // Add the irc poll descriptors to the end
    s.add_poll_descriptors<zmq_pollitem_adapter, ZMQ_POLLIN, ZMQ_POLLOUT>
      (std::back_inserter(pollfds));

    // Block for events
    zmq::poll(pollfds.data(), pollfds.size(), -1);

    // Process zmq poll descriptors
    //mhserv.process_zmq_pollfds(pollfds.begin(), pollfds.begin() + irc_begin_nth);

    // Process irc poll descriptors
    s.process_poll_descriptors<ZMQ_POLLIN, ZMQ_POLLOUT>
      (pollfds.begin() + irc_begin_nth, pollfds.end());
  }
}
