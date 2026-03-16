g++ -std=c++17 client.cpp ../packet.cpp -o client

./client --target-ip 127.0.0.1 --target-port 8888 --timeout 10 --max-retries 3
