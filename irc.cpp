#include "ircpp.hpp"

#include <cassert>
#include <cstring>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <map>

struct MyBot : public IRCSession {
  MyBot(char const* servername, unsigned int port, char const*
        password, const char* nick_, const char* username, const char*
        realname)
    : IRCSession{servername, port, password, nick_, username, realname},
    snacks{3},
    nick{nick_} { }

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
      if (snacks < 2) {
        msg(channel, "Sorry, I don't have enough snacks.");
        return;
      } else {
        snacks -= 2;
      }
      static const std::vector<std::string> botsnacks
      { "&botsnack", "~jumpsnack", ".botsnack", "+botsnack",
          "~botsnack", "^botsnack" };
      
      msg(channel, botsnacks[rand() % botsnacks.size()]);
      msg(channel, botsnacks[rand() % botsnacks.size()]);
    }
    else if (m == "-SOURCE" or m == "-source")
    {
      msg(channel, "https://github.com/ras0219/zinc");
    }
    else if (m == "-botsnack" or m == "-BOTSNACK")
    {
      snacks++;
      std::stringstream ss;
      ss << "Thanks for the snack! I now have " << snacks << " snack";
      if (snacks > 1) ss << "s."; else ss << ".";
      msg(channel, ss.str());
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
  unsigned int snacks;
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
