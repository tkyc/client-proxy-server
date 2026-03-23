g++ -std=c++17 -Wall -Wextra -O2 client.cpp ../packet.cpp ../logger.cpp ../common.cpp -o client

./client --target-ip 127.0.0.1 --target-port 4000 --timeout 5 --max-retries 3

