#include "ircpp.hpp"

extern "C" {
#include <poll.h>
}

#include <cassert>
#include <cstring>
/*#include <type>*/
#include <iostream>
#include <iomanip>
#include "zmq.hpp"
#include <sstream>
#include <stdexcept>
#include <vector>
#include <deque>
#include <map>

template<class MessageType>
struct ZMQService {
  ZMQService(zmq::context_t& ctx) : socket(ctx, ZMQ_REQ) {}

  template<class OutputIterator>
  void add_to_zmq_pollfds(OutputIterator begin) {
    short flags = ZMQ_POLLIN;
    if (!queue.empty())
      flags |= ZMQ_POLLOUT;
    *begin = { socket, 0, flags, 0 };
    ++begin;
  }

  template<class InputIterator>
  void process_zmq_pollfds(InputIterator begin, InputIterator end) {
    while (begin != end) {
      if (socket != begin->socket)
        continue;
      if (begin->revents & ZMQ_POLLIN) {
        zmq::message_t msg;
        socket.recv(&msg);
        response(queue.front(), msg);
        queue.pop_front();
      } else if (begin->revents & ZMQ_POLLOUT) {
        // Should never be empty at this time
        assert(!queue.empty());
        zmq::message_t msg = request(queue.front());
        socket.send(msg);
      } else if (begin->revents & ZMQ_POLLERR) {
        // Uhhh..... not sure what to do here halp
        throw std::runtime_error{"Halp. ZMQ_POLLERR."};
      }

      ++begin;
    }
  }

  void send(const MessageType& m) { queue.push_back(m); }
  template<class ... Args>
  void send(Args&&... args) { queue.emplace_back(args...); }

  virtual zmq::message_t request(const MessageType&) = 0;
  virtual void response(const MessageType&, zmq::message_t&) = 0;

  zmq::socket_t socket;
  std::deque<MessageType> queue;
};

enum megahal_cmd { LEARN, REPLY };

typedef std::tuple<std::string,
                   megahal_cmd,
                   std::function< void(const std::string&) > > st_fn_pair;

struct MegaHalService : ZMQService<st_fn_pair> {
  MegaHalService(zmq::context_t& ctx, const char* addr)
    : ZMQService<st_fn_pair>(ctx)
    {
      socket.connect(addr);
    }

  virtual zmq::message_t request(const st_fn_pair& m) {
    const std::string& st = std::get<0>(m);
    zmq::message_t msg(st.size() + 3);
    if (std::get<1>(m) == LEARN)
      memcpy(static_cast<char*>(msg.data()), "LRN", 3);
    else
      memcpy(static_cast<char*>(msg.data()), "REP", 3);
    memcpy(static_cast<char*>(msg.data()) + 3, st.data(), st.size());
    return msg;
  }
  virtual void response(const st_fn_pair& m, zmq::message_t& msg) {
    std::string st((const char*)msg.data(), msg.size());
    std::get<2>(m)(st);
  }
};

struct MyBot : public IRCSession {
  MyBot(char const* servername, unsigned int port, char const*
        password, const char* nick_, const char* username, const char*
        realname, ZMQService<st_fn_pair>& srv)
    : IRCSession{servername, port, password, nick_, username, realname},
    snacks{3},
    nick{nick_},
    privmsg_srv(srv)
    { }

  virtual void on_connect() {
    std::cout << "MyBot Connected.\n";
    join("#bottest");
  }
  virtual void on_nick(string_t origin, string_t nick) { }
  virtual void on_quit(string_t origin, string_t reason) { }
  virtual void on_join(string_t origin, string_t channel) {
    if (origin == nick) {
      // I have successfully joined a channel!
      // msg(channel, "Fear not, I have arrived.");

      // do... something?
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
    } else if (m.substr(0, nick.size() + 2) == nick + ": ") {
      // I'm hilightted! I'm special! Let's give them a special treat!
      privmsg_srv.send(m.substr(nick.size() + 2), REPLY, [=](const std::string& reply) {
          msg(channel, reply);
        });
    } else {
      // Message wasn't a command. We should remove highlights before learning...
      // On second thought that's hard.... forget about it
      if (m.size() == 0)
        return;
      if (m[0] == '<') {
        auto it = m.begin();
        while (it != m.end() && *it != '>') ++it;
        if (it != m.end()) ++it;
        if (it != m.end()) ++it;
        std::string new_msg = std::string(it, m.end());
        privmsg_srv.send(new_msg, LEARN, [=](const std::string& reply) {});
      } else {
        privmsg_srv.send(m, LEARN, [=](const std::string& reply) {});
      }
    }
  }
  virtual void on_privmsg(string_t origin, string_t yourname, string_t m) {
    try {
      std::cout << "<" << origin << "> " << m << "\n";

      privmsg_srv.send(m, REPLY, [=](const std::string& reply) {
          msg(origin, reply);
        });
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

private:
  unsigned int snacks;
  std::string nick;
  ZMQService<st_fn_pair>& privmsg_srv;
};

struct zmq_pollitem_adapter {
  int fd;
  short events;
  short revents;
  operator zmq::pollitem_t() { return {0, fd, events, revents}; }
};

int main(int argc, const char** argv) {
  zmq::context_t context(1);

  MegaHalService mhserv(context, "tcp://localhost:5555");

  MyBot s{"0xkohen.com", 20158, (argc > 1)?argv[argc-1]:NULL,
      "rasalghul", "rasalghul", "Ra's al Ghul", mhserv};

  std::vector<zmq::pollitem_t> pollfds;

  while (true) {
    pollfds.clear();

    std::cerr << "Adding events.\n";

    // Add the zmq poll descriptors to the beginning
    mhserv.add_to_zmq_pollfds(std::back_inserter(pollfds));

    // Mark where irc's pollfds begin
    auto irc_begin_nth = pollfds.size();

    // Add the irc poll descriptors to the end
    s.add_poll_descriptors<zmq_pollitem_adapter, ZMQ_POLLIN, ZMQ_POLLOUT>
      (std::back_inserter(pollfds));

    for (auto fd : pollfds) {
      std::cerr << std::setw(15) << fd.socket
                << std::setw(10) << fd.fd
                << std::setw(10) << fd.events
                << "\n";
    }

    std::cerr << "Waiting for events.\n";

    // Block for events
    zmq::poll(pollfds.data(), pollfds.size(), -1);

    std::cerr << "Proccessing events.\n";

    // Process zmq poll descriptors
    mhserv.process_zmq_pollfds(pollfds.begin(), pollfds.begin() + irc_begin_nth);

    // Process irc poll descriptors
    s.process_poll_descriptors<ZMQ_POLLIN, ZMQ_POLLOUT>
      (pollfds.begin() + irc_begin_nth, pollfds.end());
  }
}
