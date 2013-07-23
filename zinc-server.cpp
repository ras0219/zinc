#include <iostream>
#include "zmq.hpp"
#include <unistd.h>

int main() {
  std::cout << "Running Server." << std::endl;


  zmq::context_t context(1);
  zmq::socket_t socket(context, ZMQ_REP);

  socket.bind("tcp://*:5555");

  std::cout << "Enter loop." << std::endl;
  while (true) {
    zmq::message_t req;
    socket.recv(&req);
    std::cout << "Received message" << std::endl;

    // Work time
    sleep(1);

    zmq::message_t reply(5);
    memcpy ((void*) reply.data (), "World", 5);
    socket.send(reply);
  }

  return 0;
}
