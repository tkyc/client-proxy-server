g++ -std=c++17 server.cpp ../packet.cpp ../logger.cpp -o server

./server --listen-ip 127.0.0.1 --listen-port 5000

