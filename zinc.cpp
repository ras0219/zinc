#include <iostream>
#include "zmq.hpp"
#include <unistd.h>

int main() {
  std::cout << "Running Client." << std::endl;


  zmq::context_t context(1);
  zmq::socket_t socket(context, ZMQ_REQ);


  socket.connect("tcp://localhost:5555");

  for (auto x=0;x<10;++x) {
    zmq::message_t req(6);
    memcpy ((void*) req.data(), "Hello", 5);
    socket.send(req);

    zmq::message_t reply;
    socket.recv(&reply);
    std::cout << "Received message" << std::endl;

    // Work time
    sleep(1);

  }

  return 0;
}
