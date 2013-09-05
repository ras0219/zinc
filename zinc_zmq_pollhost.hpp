#ifndef _ZINC_ZMQ_POLLHOST_HPP_
#define _ZINC_ZMQ_POLLHOST_HPP_

#include <zmq.hpp>
#include <zinc_pollhost>
#include <vector>

namespace zinc {

  struct zmq_pollitem_adapter {
    int fd;
    short events;
    short revents;
    operator zmq::pollitem_t() { return {0, fd, events, revents}; }
  };

  struct ZMQ_PollHost {
    ZMQ_PollHost()
      : pollhost(PollHost::Impl<ZMQ_PollHost, offsetof(ZMQ_PollHost, pollhost)>::interface)
      { }

    int register_zmq(zmq::pollitem_t fd, zmq_poll_cb cb);
    int unregister_zmq(zmq::pollitem_t fd);

    void poll_once(long timeout = -1);

    PollHost::Interface pollhost;
    std::vector<zmq::pollitem_t> pollfds;
    std::vector<zmq_poll_cb> pollcbs;
  };

}

#endif
