multiplexing_with_lenght
========================

University project for Computer Networks.

The scope of this project is minimize the delay throught N sender to N recv during the transport of "big" data throught a tunnel

Multiplexing the messages and serialize them is the way. 

In the code:
mit.c: program that generate N sender with thread and send random sized messages.

TunnelTX.c: program that recive the messages from all sender and multiplex it into the socket of tunnelRX.

TunnelRX.c: program that receive all serialized messages from TunnelTX and send to right recv, after recompose them as the original.

ric.c: program that simulate the N recv with thread. Thay have to connect with the TunnelRX and wait for messages.
