#include <zinc_pollhost>
#include "zinc_zmq_pollhost.hpp"
#include <algorithm>
#include <functional>

using namespace std::placeholders;

namespace zinc {
  
  bool pollitem_cmp(const zmq::pollitem_t& lhs, const zmq::pollitem_t& rhs)
  {
    return lhs.socket < rhs.socket || (lhs.socket == rhs.socket && lhs.fd < rhs.fd);
  }

  bool pollitem_eq(const zmq::pollitem_t& lhs, const zmq::pollitem_t& rhs)
  {
    return lhs.socket == rhs.socket && lhs.fd == rhs.fd;
  }

  void ZMQ_PollHost::poll_once(long timeout) {
    int errcode = zmq::poll(pollfds.data(), pollfds.size(), timeout);

    if (errcode == 0 || errcode == EINTR)
      return;

    if (errcode < 0)
      throw std::runtime_error("error in zmq::poll");

    for (size_t x = 0; x < pollfds.size(); ++x)
      if (pollfds[x].revents != 0)
        pollcbs[x](pollfds[x]);
  }

  int ZMQ_PollHost::register_zmq(zmq::pollitem_t fd, zmq_poll_cb cb) {
    pollfds.push_back(fd);
    pollcbs.push_back(cb);
    return 0;
  }

  int ZMQ_PollHost::unregister_zmq(zmq::pollitem_t fd) {
    auto it = find_if(pollfds.begin(), pollfds.end(), std::bind(pollitem_eq, fd, _1));
    if (it == pollfds.end())
      return -1;
    auto cb_it = it - pollfds.begin() + pollcbs.begin();
    std::swap(*it, pollfds.back());
    std::swap(*cb_it, pollcbs.back());
    pollfds.pop_back();
    pollcbs.pop_back();
    return 0;
  }

}
