g++ -std=c++17 -Wall -Wextra -O2 server.cpp ../packet.cpp ../logger.cpp ../common.cpp -o server

./server --listen-ip 127.0.0.1 --listen-port 5000

