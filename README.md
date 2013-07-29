zinc
====

Zeromq + IRC. Hilarity ensues.

### Configuring Zinc
Certain configurations must currently be done by editing the source file irc.cpp. Specifically, you probably want to change the irc server address.

### Building Zinc
Currently, zinc is pre-alpha and does not use zmq. However, zmq dependency is planned. We recommend installing zmq through your favorite package manager or through the ZeroMQ website ([here](http://zeromq.org/intro:get-the-software)).

Once ZeroMQ has been taken care of, you need to build libircclient. Do this by entering the lib/libircclient-1.7 directory and building as per usual packages.

> $ cd lib/libircclient-1.7
>
> $ ./configure
>
> ...
>
> $ make

Once you have built libircclient, you can now build the rest of Zinc. In the main directory, execute

> $ make -k

### Running Zinc
Zinc is currently run by executing the command

> $ ./irc [password]

If a password is provided, it will be passed to the server via the PASS command (this differs from any nickserv passwords).
