all:server client

server:server.o
	g++ server.o -o server -L/home/scf-22/csci551b/openssl/lib -g -Wall -lsocket -lnsl -lresolv -lcrypto

client:client.o
	g++ client.o -o client -L/home/scf-22/csci551b/openssl/lib -g -Wall -lsocket -lnsl -lresolv -lcrypto 

server.o:server.cc
	g++ -I/home/scf-22/csci551b/openssl/include -c server.cc

client.o:client.cc
	g++ -I/home/scf-22/csci551b/openssl/include -c client.cc

clean:
	-rm server client *.o
