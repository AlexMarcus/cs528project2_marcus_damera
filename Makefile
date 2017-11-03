NODESOURCES=node.cpp network_node.cpp
CLIENTSOURCES=client.cpp
CC=g++
EXEC=node client
LDFLAGS= -pthread

all: $(EXEC)

node: $(NODESOURCES)
	$(CC)  $(LDFLAGS) $^ -o $@

client: $(CLIENTSOURCES)
	$(CC)  $(LDFLAGS) $^ -o $@

clean:
	rm -f *~ $(EXEC)
