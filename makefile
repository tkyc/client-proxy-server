CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
TARGETS  = client server proxy
 
.PHONY: all clean
 
all: $(TARGETS)
 
client: /client/client.cpp common.h logger.h packet.h
	$(CXX) $(CXXFLAGS) -o $@ $<
 
server: /server/server.cpp common.h logger.h packet.h
	$(CXX) $(CXXFLAGS) -o $@ $<
 
proxy: /proxy/proxy.cpp common.h logger.h packet.h
	$(CXX) $(CXXFLAGS) -o $@ $<
 
clean:
	rm -f $(TARGETS)
 
