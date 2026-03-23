CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

HEADERS     = common.h logger.h packet.h
COMMON_OBJS = common.o logger.o packet.o

TARGETS = client_bin server_bin proxy_bin

.PHONY: all clean

all: $(TARGETS)

common.o: common.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c common.cpp -o common.o

logger.o: logger.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c logger.cpp -o logger.o

packet.o: packet.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c packet.cpp -o packet.o

client_bin: client/client.cpp $(COMMON_OBJS) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ client/client.cpp $(COMMON_OBJS)

server_bin: server/server.cpp $(COMMON_OBJS) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ server/server.cpp $(COMMON_OBJS)

proxy_bin: proxy/proxy.cpp $(COMMON_OBJS) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ proxy/proxy.cpp $(COMMON_OBJS)

clean:
	rm -f $(TARGETS) $(COMMON_OBJS)

