###### Compile command
make clean && make

###### Example run commands
./client_bin --target-ip 127.0.0.1 --target-port 4000 --timeout 5 --max-retries 3
<br>
./server_bin --listen-ip 127.0.0.1 --listen-port 5000
<br>
./proxy_bin --listen-ip 127.0.0.1 --listen-port 4000 --target-ip 127.0.0.1 --target-port 5000 --client-drop 10 --server-drop 5 --client-delay 20 --server-delay 15 --client-delay-time-min 100 --client-delay-time-max 200 --server-delay-time-min 150 --server-delay-time-max 300

