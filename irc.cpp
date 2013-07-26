extern "C" {
#include "libircclient.h"
#include "libirc_rfcnumeric.h"
}
#include <cassert>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <map>

struct IRCSession {
  irc_session_t* session;

  IRCSession(char const* servername,
             unsigned int port,
             char const* password,
             const char* nick,
             const char* username,
             const char* realname);

  virtual ~IRCSession();

  using string_t = const std::string&;

  // Methods to be overloaded by subclasses
  virtual void on_connect() { }
  virtual void on_nick(string_t origin, string_t nick) { }
  virtual void on_quit(string_t origin, string_t reason) { }
  virtual void on_join(string_t origin, string_t channel) { }
  virtual void on_part(string_t origin, string_t channel, string_t reason) { }
  virtual void on_channel(string_t origin, string_t channel, string_t msg) { }
  virtual void on_privmsg(string_t origin, string_t yourname, string_t msg) { }
  virtual void on_numeric(unsigned int event, char const * origin,
                          char const ** params, unsigned int count) {}

  // methods used by subclasses
  void join(const std::string &channel) {
    if (irc_cmd_join( session, channel.c_str(), 0 ))
      throw std::runtime_error{"Could not join channel."};
  }
  void join(const std::string& channel, const std::string& password) {
    if (irc_cmd_join( session, channel.c_str(), password.c_str()))
      throw std::runtime_error{"Could not join channel."};
  }
  void part(const std::string& channel) {
    if (irc_cmd_part( session, channel.c_str()))
      throw std::runtime_error{"Could not part channel."};
  }
  void msg(const std::string& target, const std::string& m) {
    if (irc_cmd_msg( session, target.c_str(), m.c_str()))
      throw std::runtime_error{"Could not send message."};
  }
  void me(const std::string& target, const std::string& m) {
    if (irc_cmd_me( session, target.c_str(), m.c_str()))
      throw std::runtime_error{"Could not send message."};
  }
  void invite(const std::string& nick, const std::string& channel) {
    if (irc_cmd_invite( session, nick.c_str(), channel.c_str()))
      throw std::runtime_error{"Could not send invite."};
  }
  void names(const std::string& channel) {
    if (irc_cmd_names( session, channel.c_str()))
      throw std::runtime_error{"Could not get names."};
  }
  void disconnect() {
    irc_disconnect(session);
  }


  // Used when there is only one session and we want the default handler

  void run() {
    if (irc_run(session))
      throw std::runtime_error{"Could not start main irc listening loop."};
  }
};


struct IRC {
  // All the callbacks.... :<
  static void event_connect(struct irc_session_s* sess, char const *
                            event, char const * origin, char const **
                            params, unsigned int count)
    {
      auto irc_s = session_map.find(sess);
      if (irc_s != session_map.end())
        irc_s->second->on_connect();
    }

  static void event_nick(struct irc_session_s* sess, char const *
                         event, char const * origin, char const **
                         params, unsigned int count)
    {
      assert(count == 1);
      auto irc_s = session_map.find(sess);
      if (irc_s != session_map.end())
        irc_s->second->on_nick(origin, params[0]);
    }

  static void event_quit(struct irc_session_s* sess, char const *
                            event, char const * origin, char const **
                            params, unsigned int count)
    {
      auto irc_s = session_map.find(sess);
      if (irc_s != session_map.end())
        if (count == 1)
          irc_s->second->on_quit(origin, params[0]);
        else
          irc_s->second->on_quit(origin, "");
    }

  static void event_join(struct irc_session_s* sess, char const *
                         event, char const * origin, char const **
                         params, unsigned int count)
    {
      auto irc_s = session_map.find(sess);
      if (irc_s != session_map.end())
        irc_s->second->on_join(origin, params[0]);
    }

  static void event_channel(struct irc_session_s* sess, char const *
                            event, char const * origin, char const **
                            params, unsigned int count)
    {
      auto irc_s = session_map.find(sess);
      if (irc_s != session_map.end())
        if (count == 2)
          irc_s->second->on_channel(origin, params[0], params[1]);
        else
          irc_s->second->on_channel(origin, params[0], "");
    }

  static void event_privmsg(struct irc_session_s* sess, char const *
                            event, char const * origin, char const **
                            params, unsigned int count)
    {
      auto irc_s = session_map.find(sess);
      if (irc_s != session_map.end())
        if (count == 2)
          irc_s->second->on_privmsg(origin, params[0], params[1]);
        else
          irc_s->second->on_privmsg(origin, params[0], "");
    }

  static void event_numeric(struct irc_session_s* sess, unsigned int
                            event, char const * origin, char const **
                            params, unsigned int count)
    {
      auto irc_s = session_map.find(sess);
      if (irc_s != session_map.end())
        irc_s->second->on_numeric(event, origin, params, count);
    }

  // Static member tables
  static irc_callbacks_t callbacks;
  using session_map_t = std::map<irc_session_t*,IRCSession*>;
  static session_map_t session_map;
};

irc_callbacks_t IRC::callbacks{0};
IRC::session_map_t IRC::session_map{};


IRCSession::IRCSession(char const* servername,
                       unsigned int port,
                       char const* password,
                       const char* nick,
                       const char* username,
                       const char* realname)
  : session{nullptr}
    {
      session = irc_create_session(&IRC::callbacks);
      
      if (session == nullptr)
        throw std::runtime_error{"Could not create IRC session."};

      irc_option_set( session, LIBIRC_OPTION_STRIPNICKS );

      if (irc_connect(session, servername, port, password, nick,
                      username, realname))
        throw std::runtime_error{"Could not initiate connection to server."};

      IRC::session_map[session] = this;
    }

IRCSession::~IRCSession() {
  irc_disconnect(session);
  IRC::session_map.erase(session);
}



struct MyBot : public IRCSession {
  MyBot(char const* servername, unsigned int port, char const*
        password, const char* nick_, const char* username, const char*
        realname)
    : IRCSession{servername, port, password, nick_, username, realname}, nick{nick_} { }

  virtual void on_connect() {
    std::cout << "MyBot Connected.\n";
    join("#bottest");
  }
  virtual void on_nick(string_t origin, string_t nick) { }
  virtual void on_quit(string_t origin, string_t reason) { }
  virtual void on_join(string_t origin, string_t channel) {
    if (origin == nick) {
      // I have successfully joined a channel!
      msg(channel, "Fear not, I have arrived.");
    }
  }
  virtual void on_part(string_t origin, string_t channel, string_t reason) { }
  virtual void on_channel(string_t origin, string_t channel, string_t m) {
    std::cout << "MyBot Received Channel Message from:\n"
              << "[" << channel << "] <" << origin << "> \""
              << m << "\"\n";

    if (m == "-SNACKTIME" or m == "-snacktime")
    {
      msg(channel, "&botsnack");
      msg(channel, "^botsnack");
      msg(channel, "~jumpsnack");
      msg(channel, "Destult: botsnack");
    }
    if (m == "-SOURCE" or m == "-source")
    {
      msg(channel, "https://github.com/ras0219/zinc");
    }

  }
  virtual void on_privmsg(string_t origin, string_t yourname, string_t m) {
    if (m.substr(0,5) == "join ")
      join(m.substr(5));
  }

  virtual void on_numeric(unsigned int event, char const * origin, char const
                  ** params, unsigned int count) {
    std::cout << "MyBot Received Numeric Message.\n";
    for (unsigned int n = 0; n < count; ++n) {
      std::cout << params[n] << "\n";
    }
  }

private:
  std::string nick;
};

int main(int argc, const char** argv) {
  IRC::callbacks.event_connect = IRC::event_connect;
  IRC::callbacks.event_nick    = IRC::event_nick;
  IRC::callbacks.event_quit    = IRC::event_quit;
  IRC::callbacks.event_channel = IRC::event_channel;
  IRC::callbacks.event_privmsg = IRC::event_privmsg;
  IRC::callbacks.event_numeric = IRC::event_numeric;

  //MyBot s{"irc.tamu.edu", 6667, 0, "rasalghul", "rasalghul", "Ra's al Ghul"};
  MyBot s{"0xkohen.com", 20158, (argc > 1)?argv[argc-1]:NULL,
      "rasalghul", "rasalghul", "Ra's al Ghul"};
  s.run();
}
