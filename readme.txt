client.c copies the contents from a .txt file named beforeTransfer.txt, which is
in the same directory.
    - client takes arguments :
    ./client <server address(which is on the same subnet)> <PORT>

server.c demonstrates unreliable communication through utilizing sequence numbers
and the packet struct, and copies the contents of the file sent by the client and
writes them into a text file which is called afterTransfer.txt.
    - server takes arguments:
    ./server <PORT> <LOSS %>
                -loss % ranges from 0.0 - 0.99

By default, two packets of 500 bytes are sent at a time. And the max number
of bytes per file is 4096 bytes

to compile : make all
