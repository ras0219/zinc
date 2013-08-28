#include <iostream>
#include "zmq.hpp"
#include <unistd.h>
#include <cstring>
#include <csignal>

extern "C" {
#include "megahal.h"
}

void sig_handler(int signo)
{
  if (signo == SIGINT)
    megahal_cleanup();
  exit(0);
}

void init_megahal() {
  megahal_setnoprompt();
  megahal_setnobanner();

  megahal_setdirectory((char*)"/Users/ras0219/projects/zinc");

  megahal_initialize();
}

int main() {
  // std::cout << "Hello, world!" << std::endl;
  if (signal(SIGINT, sig_handler) == SIG_ERR)
    std::cerr << "Couldn't intercept signal." << std::endl;

  unsigned int msg_counter = 0;

  init_megahal();

  zmq::context_t context(1);
  zmq::socket_t socket(context, ZMQ_REP);

  socket.bind("tcp://*:5555");

  while (true) {
    zmq::message_t req;
    socket.recv(&req);
    std::cout << "Received message" << std::endl;

    if (req.size() < 3) {
      // No command, reject
      socket.send("FAIL", 4);
    } else if (memcmp(req.data(), "REP", 3) == 0) {
      // Response requested
      std::string data{static_cast<char*>(req.data()) + 3, req.size() - 3};
      char * resp = megahal_do_reply((char*)data.c_str(), 0);
      socket.send(resp, strlen(resp));
    } else if (memcmp(req.data(), "LRN", 3) == 0) {
      // No response requested, just learn
      std::string data{static_cast<char*>(req.data()) + 3, req.size() - 3};
      megahal_learn_no_reply((char*)data.c_str(), 0);
      socket.send("OK", 2);
    } else {
      // Invalid command, reject
      socket.send("FAIL", 4);
    }

    std::cout << "Reply sent" << std::endl;

    if (++msg_counter % 10 == 0)
      megahal_save();
  }

  megahal_cleanup();

  return 0;
}
