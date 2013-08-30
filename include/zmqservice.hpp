#ifndef _ZMQSERVICE_HPP_
#define _ZMQSERVICE_HPP_

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
  // template<class ... Args>
  // void send(Args&&... args) { queue.emplace_back(args...); }

  virtual zmq::message_t request(const MessageType&) = 0;
  virtual void response(const MessageType&, zmq::message_t&) = 0;

  zmq::socket_t socket;
  std::deque<MessageType> queue;
};

#endif
