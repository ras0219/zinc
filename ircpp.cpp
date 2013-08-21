#include "ircpp.hpp"

#include <cassert>
#include <cstring>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <map>


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

// Methods to be overloaded by subclasses
void IRCSession::on_connect() { }
void IRCSession::on_nick(string_t origin, string_t nick) { }
void IRCSession::on_quit(string_t origin, string_t reason) { }
void IRCSession::on_join(string_t origin, string_t channel) { }
void IRCSession::on_part(string_t origin, string_t channel, string_t reason) { }
void IRCSession::on_channel(string_t origin, string_t channel, string_t msg) { }
void IRCSession::on_privmsg(string_t origin, string_t yourname, string_t msg) { }
void IRCSession::on_numeric(unsigned int event, char const * origin,
                            char const ** params, unsigned int count) {}

// methods used by subclasses
void IRCSession::join(const std::string &channel) {
  if (irc_cmd_join( session, channel.c_str(), 0 ))
    throw std::runtime_error{"Could not join channel."};
}
void IRCSession::join(const std::string& channel, const std::string& password) {
  if (irc_cmd_join( session, channel.c_str(), password.c_str()))
    throw std::runtime_error{"Could not join channel."};
}
void IRCSession::part(const std::string& channel) {
  if (irc_cmd_part( session, channel.c_str()))
    throw std::runtime_error{"Could not part channel."};
}
void IRCSession::msg(const std::string& target, const std::string& m) {
  if (irc_cmd_msg( session, target.c_str(), m.c_str()))
    throw std::runtime_error{"Could not send message."};
}
void IRCSession::me(const std::string& target, const std::string& m) {
  if (irc_cmd_me( session, target.c_str(), m.c_str()))
    throw std::runtime_error{"Could not send message."};
}
void IRCSession::invite(const std::string& nick, const std::string& channel) {
  if (irc_cmd_invite( session, nick.c_str(), channel.c_str()))
    throw std::runtime_error{"Could not send invite."};
}
void IRCSession::names(const std::string& channel) {
  if (irc_cmd_names( session, channel.c_str()))
    throw std::runtime_error{"Could not get names."};
}
void IRCSession::disconnect() {
  irc_disconnect(session);
}

// Used when there is only one session and we want the default handler
void IRCSession::run() {
  if (irc_run(session))
    throw std::runtime_error{"Could not start main irc listening loop."};
}


// All the callbacks.... :<
void IRC::event_connect(struct irc_session_s* sess, char const *
                        event, char const * origin, char const **
                        params, unsigned int count)
{
  auto irc_s = session_map.find(sess);
  if (irc_s != session_map.end())
    irc_s->second->on_connect();
}

void IRC::event_nick(struct irc_session_s* sess, char const *
                     event, char const * origin, char const **
                     params, unsigned int count)
{
  assert(count == 1);
  auto irc_s = session_map.find(sess);
  if (irc_s != session_map.end())
    irc_s->second->on_nick(origin, params[0]);
}

void IRC::event_quit(struct irc_session_s* sess, char const *
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

void IRC::event_join(struct irc_session_s* sess, char const *
                     event, char const * origin, char const **
                     params, unsigned int count)
{
  auto irc_s = session_map.find(sess);
  if (irc_s != session_map.end())
    irc_s->second->on_join(origin, params[0]);
}

void IRC::event_channel(struct irc_session_s* sess, char const *
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

void IRC::event_privmsg(struct irc_session_s* sess, char const *
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

void IRC::event_numeric(struct irc_session_s* sess, unsigned int
                        event, char const * origin, char const **
                        params, unsigned int count)
{
  auto irc_s = session_map.find(sess);
  if (irc_s != session_map.end())
    irc_s->second->on_numeric(event, origin, params, count);
}

irc_callbacks_t IRC::callbacks{
  IRC::event_connect, // irc_event_callback_t event_connect;
  IRC::event_nick, // irc_event_callback_t event_nick;
  IRC::event_quit, // irc_event_callback_t event_quit;
  IRC::event_join, // irc_event_callback_t event_join;
  IRC::event_part, // irc_event_callback_t event_part;
  IRC::event_mode, // irc_event_callback_t event_mode;
  IRC::event_umode, // irc_event_callback_t event_umode;
  IRC::event_topic, // irc_event_callback_t event_topic;
  IRC::event_kick, // irc_event_callback_t event_kick;
  IRC::event_channel, // irc_event_callback_t event_channel;
  IRC::event_privmsg, // irc_event_callback_t event_privmsg;
  IRC::event_notice, // irc_event_callback_t event_notice;
  IRC::event_channel_notice, // irc_event_callback_t event_channel_notice;
  IRC::event_invite, // irc_event_callback_t event_invite;
  IRC::event_ctcp_req, // irc_event_callback_t event_ctcp_req;
  IRC::event_ctcp_rep, // irc_event_callback_t event_ctcp_rep;
  IRC::event_ctcp_action, // irc_event_callback_t event_ctcp_action;
  IRC::event_unknown, // irc_event_callback_t event_unknown;
  IRC::event_numeric, // irc_eventcode_callback_t event_numeric;
  IRC::event_dcc_chat_req, // irc_event_dcc_chat_t event_dcc_chat_req;
  IRC::event_dcc_send_req // irc_event_dcc_send_t event_dcc_send_req;
};
IRC::session_map_t IRC::session_map{};
