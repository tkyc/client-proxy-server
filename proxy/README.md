g++ -std=c++17 proxy.cpp ../packet.cpp ../logger.cpp -o proxy

./proxy --listen-ip 127.0.0.1 --listen-port 4000 --target-ip 127.0.0.1 --target-port 5000 --client-drop 10 --server-drop 5 --client-delay 20 --server-delay 15 --client-delay-time-min 100 --client-delay-time-max 200 --server-delay-time-min 150 --server-delay-time-max 300

