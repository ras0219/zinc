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

enum megahal_cmd { LEARN, REPLY };
struct MessageT {
  std::string msg;
  megahal_cmd cmd;
  std::function< void(const std::string&) > cb;
};

struct MegaHalService : ZMQService<MessageT> {
  MegaHalService(zmq::context_t& ctx, const char* addr)
    : ZMQService<MessageT>(ctx)
    {
      socket.connect(addr);
    }

  virtual zmq::message_t request(const MessageT& m) {
    zmq::message_t msg(m.msg.size() + 3);
    if (m.cmd == LEARN)
      memcpy(static_cast<char*>(msg.data()), "LRN", 3);
    else
      memcpy(static_cast<char*>(msg.data()), "REP", 3);
    memcpy(static_cast<char*>(msg.data()) + 3, m.msg.data(), m.msg.size());
    return msg;
  }
  virtual void response(const MessageT& m, zmq::message_t& msg) {
    std::string st((const char*)msg.data(), msg.size());
    m.cb(st);
  }
};

struct IRCChannelContext : pnp::ContextThunk<IRCChannelContext> {
  IRCChannelContext(str_t channel, IRCSession& mb) : chan(channel), irc(mb) {}
  
  void reply(str_t msg) {
    irc.msg(chan, msg);
  }
  void irc_join(str_t channel) {
    irc.join(channel);
  }
  void irc_msg(str_t target, str_t msg) {
    irc.msg(target, msg);
  }
  
  str_t chan;
  IRCSession& irc;
};


struct MyBot : public IRCSession {
  MyBot(char const* servername, unsigned int port, char const*
        password, const char* nick_, const char* username, const char*
        realname, ZMQService<MessageT>& srv)
    : IRCSession{servername, port, password, nick_, username, realname},
    snacks{3},
    nick{nick_},
    privmsg_srv(srv)
    { }

  virtual void on_connect() {
    std::cout << "MyBot Connected.\n";
  }
  virtual void on_nick(string_t origin, string_t nick) { }
  virtual void on_quit(string_t origin, string_t reason) { }
  virtual void on_join(string_t origin, string_t channel) {
    if (origin == nick) {
      // I have successfully joined a channel!
      // msg(channel, "Fear not, I have arrived.");

      // do... something?
      std::cout << "Succesfully joined <" << channel << ">" << std::endl;
    } else {
      std::cout << origin << " succesfully joined <" << channel << ">" << std::endl;
    }
  }
  virtual void on_part(string_t origin, string_t channel, string_t reason) { }
  virtual void on_channel(string_t origin, string_t channel, string_t m) {
    std::cout << "[" << channel << "] <" << origin << "> \""
              << m << "\"" << std::endl;

    if (m == nullptr)
      // Ignore empty messages
      return;

    if (m[0] == '-') {
      // Process a command
      const char* eoc = strchr(m, ' ');

      std::string cmd;
      if (eoc == NULL)
        cmd.assign(m + 1);
      else
        cmd.assign(m, eoc++);

      // Lowercase all commands.
      transform(cmd.begin(), cmd.end(), cmd.begin(), tolower);

      handle_command(cmd, origin, channel, eoc);
    }
    else if (strncmp(m, nick.c_str(), nick.size()) == 0
             && m[nick.size()] == ':'
             && m[nick.size() + 1] == ' ')
    {
      // I'm hilightted! I'm special! Let's give them a special treat!
      std::string newmsg = m + nick.size() + 2;
      std::cout << "<" << channel << "/" << origin << "> " << newmsg << std::endl;

      std::string channel_s = channel;
      privmsg_srv.send({newmsg, REPLY, [this,channel_s](const std::string& reply) {
            msg(channel_s.c_str(), reply.c_str());
          }});
    } else {
      // Message wasn't a command. We should remove highlights before learning...
      // On second thought that's hard.... forget about it
      if (m[0] == 0) return;

      std::string st = m;
      if (st[0] == '<') {
        auto it = st.begin();
        while (it != st.end() && *it != '>') ++it;
        if (it != st.end()) ++it;
        if (it != st.end()) ++it;
        std::string new_msg = std::string(it, st.end());
        privmsg_srv.send({new_msg, LEARN, [](const std::string& reply) {}});
      } else {
        privmsg_srv.send({st, LEARN, [](const std::string& reply) {}});
      }
    }
  }
  virtual void on_privmsg(string_t origin, string_t yourname, string_t m) {
    try {
      std::cout << "<" << origin << "> " << m << "\n";

      if (m == nullptr)
        // Ignore empty messages
        return;

      if (m[0] == '-') {
        // Process a command
        const char* eoc = strchr(m, ' ');

        std::string cmd;
        if (eoc == NULL)
          cmd.assign(m + 1);
        else
          cmd.assign(m + 1, eoc++);

        // Lowercase all commands.
        transform(cmd.begin(), cmd.end(), cmd.begin(), tolower);

        // std::cout << "[" << cmd << "] ";
        // if (eoc != nullptr)
        //   std::cout << "[" << eoc << "]";
        // else
        //   std::cout << "nullptr";
        // std::cout << std::endl;

        handle_command(cmd, origin, origin, eoc);
      } else {
        std::string origin_s = origin;
        privmsg_srv.send({m, REPLY, [this,origin_s](const std::string& reply) {
              msg(origin_s.c_str(), reply.c_str());
            }});
      }
    } catch (std::exception& e) {
      msg(origin, e.what());
    }
  }

  virtual void on_numeric(unsigned int event, char const * origin, char const
                          ** params, unsigned int count) {
    std::cout << "MyBot Received Numeric Message.\n";
    for (unsigned int n = 0; n < count; ++n) {
      std::cout << params[n] << "\n";
    }
  }

  void handle_command(const std::string& cmd, string_t origin, string_t channel, string_t rem) {
      if (cmd == "snacktime") {
        snacktime(channel);
        return;
      }

      if (cmd == "source") {
        msg(channel, "https://github.com/ras0219/zinc");
        return;
      }
      if (cmd == "botsnack") {
        snacks++;
        std::stringstream ss;
        ss << "Thanks for the snack! I now have " << snacks << " snack";
        if (snacks > 1) ss << "s."; else ss << ".";
        msg(channel, ss.str().c_str());
        return;
      }
      auto it = cmd_handlers.find(cmd);
      if (it != cmd_handlers.end()) {
        IRCChannelContext irccc(channel, *this);

        it->second(&irccc, rem);
      }
  }

  void snacktime(string_t channel) {
    if (snacks < 2) {
      msg(channel, "Sorry, I don't have enough snacks.");
      return;
    } else {
      snacks -= 2;
    }
    static std::vector<const char*> botsnacks =
      { "&botsnack", "~jumpsnack", ".botsnack", "+botsnack",
        "~botsnack", "^botsnack" };
      
    msg(channel, botsnacks[rand() % botsnacks.size()]);
    msg(channel, botsnacks[rand() % botsnacks.size()]);
  }

  std::map<std::string, pnp::command_cb> cmd_handlers;

private:
  unsigned int snacks;
  std::string nick;
  ZMQService<MessageT>& privmsg_srv;
};

struct zmq_pollitem_adapter {
  int fd;
  short events;
  short revents;
  operator zmq::pollitem_t() { return {0, fd, events, revents}; }
};

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
  {
    {L"brainsocket", L"tcp://localhost:5555"},
    {L"serveraddr", L"irc.freenode.net"},
    {L"serverpass", L""},
    {L"username", L"rasalghul"},
    {L"nickname", L"rasalghul"},
    {L"realname", L"Ra's al Ghul"}
  },
  {
    {L"serverport", 6667}
  },
  {},
  {}
  };


int main(int argc, const char** argv) {
  zmq::context_t context(1);

  if (argc > 1)
    global_config.parse_file(argv[1]);

  MegaHalService mhserv(context, global_config.get_string_config(L"brainsocket").c_str());

  string
    serverpass = global_config.get_string_config(L"serverpass"),
    serveraddr = global_config.get_string_config(L"serveraddr"),
    username = global_config.get_string_config(L"username"),
    nickname = global_config.get_string_config(L"nickname"),
    realname = global_config.get_string_config(L"realname");
  int port = global_config.get_num_config(L"serverport");

  MyBot s(serveraddr.c_str(),
          port,
          serverpass.empty() ? NULL : serverpass.c_str(),
          username.c_str(),
          nickname.c_str(),
          realname.c_str(),
          mhserv);

  pnp::ZincPluginHost zph;

  struct ZincPluginHostProxy : pnp::PluginHostThunk<ZincPluginHostProxy> {
    ZincPluginHostProxy(MyBot& m, pnp::ZincPluginHost& z) : mb(m), zph(z) {}

    int register_command(str_t cmd, pnp::command_cb ptr) {
      // actual implementation here
      if (mb.cmd_handlers.find(cmd) != mb.cmd_handlers.end())
        return -1;
      mb.cmd_handlers.insert({cmd, ptr});
      return 0;
    }

    int unregister_command(str_t cmd) {
      auto it = mb.cmd_handlers.find(cmd);
      if (it == mb.cmd_handlers.end())
        return -1;

      mb.cmd_handlers.erase(it);
      return 0;
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
      mb.join(channel);
    }

    void irc_msg(str_t target, str_t msg) {
      mb.msg(target, msg);
    }

    MyBot& mb;
    pnp::ZincPluginHost& zph;
  } zphp(s, zph);

  for (auto f : global_config.libraries_to_load) {
    cout << "loading <" << f << ">" << endl;
    zph.add_library_file(f);
  }

  for (auto p : global_config.plugs_to_load) {
    assert(zph.load_plugin(p.first, p.second, &zphp));
  }

  std::vector<zmq::pollitem_t> pollfds;

  while (true) {
    pollfds.clear();

    // Add the zmq poll descriptors to the beginning
    mhserv.add_to_zmq_pollfds(std::back_inserter(pollfds));

    // Mark where irc's pollfds begin
    auto irc_begin_nth = pollfds.size();

    // Add the irc poll descriptors to the end
    s.add_poll_descriptors<zmq_pollitem_adapter, ZMQ_POLLIN, ZMQ_POLLOUT>
      (std::back_inserter(pollfds));

    // Block for events
    zmq::poll(pollfds.data(), pollfds.size(), -1);

    // Process zmq poll descriptors
    mhserv.process_zmq_pollfds(pollfds.begin(), pollfds.begin() + irc_begin_nth);

    // Process irc poll descriptors
    s.process_poll_descriptors<ZMQ_POLLIN, ZMQ_POLLOUT>
      (pollfds.begin() + irc_begin_nth, pollfds.end());
  }
}
