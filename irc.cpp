extern "C" {
#include "libircclient.h"
#include "libirc_rfcnumeric.h"
}
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <map>

template<typename T, typename R, typename ...Args>
using mem_func_ptr = R (T::*)(Args...);

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
  virtual void on_connect() = 0;
  virtual void on_nick(string_t origin, string_t nick) { }
  virtual void on_quit(string_t origin, string_t reason) { }
  virtual void on_join(string_t origin, string_t channel) { }
  virtual void on_part(string_t origin, string_t channel, string_t reason) { }
  virtual void on_channel(string_t origin, string_t channel, string_t msg) { }
  virtual void on_numeric(unsigned int event, char const * origin,
                          char const ** params, unsigned int count)
  = 0;
    
  // methods used by subclasses
  void join(const std::string &room) {
    if (irc_cmd_join( session, room.c_str(), 0 ))
      throw std::runtime_error{"Could not join room."};
  }
  void join(const std::string& room, const std::string& password) {
    if (irc_cmd_join( session, room.c_str(), password.c_str()))
      throw std::runtime_error{"Could not join room."};
  }
  void part(const std::string& room) {
    if (irc_cmd_part( session, room.c_str()))
      throw std::runtime_error{"Could not part room."};
  }

  void msg(const std::string& room, const std::string& m) {
    if (irc_cmd_msg( session, room.c_str(), m.c_str()))
      throw std::runtime_error{"Could not send message."};
  }

  // Used when there is only one session and we want the default handler

  void run() {
    if (irc_run(session))
      throw std::runtime_error{"Could not start main irc listening loop."};
  }
};


struct IRC {
  
  using event_subhandler =
    mem_func_ptr<IRCSession, void, const char*, char const*, char const**, unsigned int>;

  // All the callbacks.... :<
  static void event_connect(struct irc_session_s* sess, char const *
                            event, char const * origin, char const **
                            params, unsigned int count)
    {
      auto irc_s = session_map.find(sess);
      if (irc_s != session_map.end())
        irc_s->second->on_connect(event, origin, params, count);
    }

  static void event_quit(struct irc_session_s* sess, char const *
                            event, char const * origin, char const **
                            params, unsigned int count)
    {
      auto irc_s = session_map.find(sess);
      if (irc_s != session_map.end())
        irc_s->second->on_quit(event, origin, params, count);
    }

  static void event_channel(struct irc_session_s* sess, char const *
                            event, char const * origin, char const **
                            params, unsigned int count)
    {
      auto irc_s = session_map.find(sess);
      if (irc_s != session_map.end())
        irc_s->second->on_channel(event, origin, params, count);
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
  IRC::session_map.erase(session);
}



struct MyBot : public IRCSession {
  MyBot(char const* servername, unsigned int port, char const*
        password, const char* nick, const char* username, const char*
        realname)
    : IRCSession{servername, port, password, nick, username, realname} { }

  virtual void on_connect(char const * event, char const * origin, char const
                          ** params, unsigned int count) {
    join("#bottest");
    std::cout << "MyBot Connected.\n";
    for (unsigned int n = 0; n < count; ++n) {
      std::cout << params[n] << "\n";
    }
  }

  virtual void on_numeric(unsigned int event, char const * origin, char const
                  ** params, unsigned int count) {
    std::cout << "MyBot Received Numeric Message.\n";
    for (unsigned int n = 0; n < count; ++n) {
      std::cout << params[n] << "\n";
    }
  }

  virtual void on_channel(char const * event, char const * origin, char const
                  ** params, unsigned int count) {
    std::cout << "MyBot Received Channel Message from:\n"
              << "<" << origin << ">\n";
    for (unsigned int n = 0; n < count; ++n) {
      std::cout << params[n] << "\n";
    }
  }

};



int main() {
  IRC::callbacks.event_connect = IRC::event_connect;
  IRC::callbacks.event_quit = IRC::event_quit;
  IRC::callbacks.event_channel = IRC::event_channel;
  IRC::callbacks.event_numeric = IRC::event_numeric;

  MyBot s{"irc.tamu.edu", 6667, 0, "raise", "raise", "none applicable"};
  s.run();
}
