#ifndef _IRCPP_HPP_
#define _IRCPP_HPP_

#include <iostream>

extern "C" {
#include "libircclient.h"
#include "libirc_rfcnumeric.h"
#include "sys/select.h"
#include "poll.h"
}
#include <string>
#include <map>

struct IRCSession {
  IRCSession(char const* servername,
             unsigned int port,
             char const* password,
             const char* nick,
             const char* username,
             const char* realname);

  virtual ~IRCSession();

  typedef const char* string_t; //const std::string&;

  // Methods to be overloaded by subclasses
  virtual void on_connect();
  virtual void on_nick(string_t origin, string_t nick);
  virtual void on_quit(string_t origin, string_t reason);
  virtual void on_join(string_t origin, string_t channel);
  virtual void on_part(string_t origin, string_t channel, string_t reason);
  virtual void on_channel(string_t origin, string_t channel, string_t msg);
  virtual void on_privmsg(string_t origin, string_t yourname, string_t msg);
  virtual void on_numeric(unsigned int event, char const * origin,
                          char const ** params, unsigned int count);

  // methods used by subclasses
  void join(string_t channel);
  void join(string_t channel, string_t password);
  void part(string_t channel);
  void msg(string_t target, string_t m);
  void me(string_t target, string_t m);
  void invite(string_t nick, string_t channel);
  void names(string_t channel);
  void disconnect();

  // Used when there is only one session and we want the default handler
  void run();

  // Fill an fdset with irc sockets
  // Basically calls irc_add_select_descriptors
  void add_select_descriptors(fd_set* in_set, fd_set* out_set, int* maxfd);

  // Similar to add_select_descriptors
  void process_select_descriptors(fd_set* in_set, fd_set* out_set);
  
  // Recommend calling this with a std::vector<struct pollfd> like
  //   ircsession.add_poll_descriptors(std::back_inserter(myvec));
  template<class OutputType = pollfd,
           short INEVENTS=POLLIN | POLLPRI,
           short OUTEVENTS=POLLOUT | POLLERR | POLLHUP,
           class OutputIterator>
  void add_poll_descriptors(OutputIterator first) {
    fd_set in_set, out_set;
    FD_ZERO(&in_set);
    FD_ZERO(&out_set);
    int maxfd = -1;

    add_select_descriptors(&in_set, &out_set, &maxfd);

    for(int x = 0; x <= maxfd; ++x) {
      short events = 0;
      if (FD_ISSET(x, &in_set)) events |= INEVENTS;
      if (FD_ISSET(x, &out_set)) events |= OUTEVENTS;
      if (events) {
        *first = OutputType({x, events, 0});
        ++first;
      }
    }
  }

  // This function is a bit inefficient if you have multiple sessions;
  // otherwise, go for it
  //
  // Ex. ircsession.process_poll_descriptors(myvec.begin(), myvec.end());
  template<short INEVENTS=POLLIN | POLLPRI,
           short OUTEVENTS=POLLOUT | POLLERR | POLLHUP,
           class InputIterator>
  void process_poll_descriptors(InputIterator begin, InputIterator end) {
    fd_set in_set, out_set;
    FD_ZERO(&in_set);
    FD_ZERO(&out_set);

    while (begin != end) {
      if (begin->fd >= 0 && (begin->revents & INEVENTS))
        FD_SET(begin->fd, &in_set);

      if (begin->fd >= 0 && (begin->revents & OUTEVENTS))
        FD_SET(begin->fd, &out_set);

      ++begin;
    }

    process_select_descriptors(&in_set, &out_set);
  }

  void resolve_errno();

  irc_session_t* session;
};

struct IRC {
  // All the callbacks.... :<
  static void event_connect(struct irc_session_s* sess, char const *
                            event, char const * origin, char const **
                            params, unsigned int count);

  static void event_nick(struct irc_session_s* sess, char const *
                         event, char const * origin, char const **
                         params, unsigned int count);

  static void event_quit(struct irc_session_s* sess, char const *
                         event, char const * origin, char const **
                         params, unsigned int count);

  static void event_join(struct irc_session_s* sess, char const *
                         event, char const * origin, char const **
                         params, unsigned int count);

  static void event_channel(struct irc_session_s* sess, char const *
                            event, char const * origin, char const **
                            params, unsigned int count);

  static void event_privmsg(struct irc_session_s* sess, char const *
                            event, char const * origin, char const **
                            params, unsigned int count);

  static void event_numeric(struct irc_session_s* sess, unsigned int
                            event, char const * origin, char const **
                            params, unsigned int count);

  // As yet unimplemented
  constexpr static irc_event_callback_t event_part = 0;
  constexpr static irc_event_callback_t event_mode = 0;
  constexpr static irc_event_callback_t event_umode = 0;
  constexpr static irc_event_callback_t event_topic = 0;
  constexpr static irc_event_callback_t event_kick = 0;
  constexpr static irc_event_callback_t event_notice = 0;
  constexpr static irc_event_callback_t event_channel_notice = 0;
  constexpr static irc_event_callback_t event_invite = 0;
  constexpr static irc_event_callback_t event_ctcp_req = 0;
  constexpr static irc_event_callback_t event_ctcp_rep = 0;
  constexpr static irc_event_callback_t event_ctcp_action = 0;
  constexpr static irc_event_callback_t event_unknown = 0;
  constexpr static irc_event_dcc_chat_t event_dcc_chat_req = 0;
  constexpr static irc_event_dcc_send_t event_dcc_send_req = 0;

  // Static member tables
  static irc_callbacks_t callbacks;

  using session_map_t = std::map<irc_session_t*,IRCSession*>;
  static session_map_t session_map;
};

#endif
