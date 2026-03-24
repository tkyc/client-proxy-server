###### Compile command
make clean && make

###### Example run commands
./client_bin --target-ip 127.0.0.1 --target-port 4000 --timeout 5 --max-retries 3
<br>
./server_bin --listen-ip 127.0.0.1 --listen-port 5000
<br>
./proxy_bin --listen-ip 127.0.0.1 --listen-port 4000 --target-ip 127.0.0.1 --target-port 5000 --client-drop 10 --server-drop 5 --client-delay 20 --server-delay 15 --client-delay-time-min 100 --client-delay-time-max 200 --server-delay-time-min 150 --server-delay-time-max 300

###### 0% Drop, 0% Delay
./client_bin --target-ip 127.0.0.1 --target-port 4000 --timeout 3 --max-retries 50
<br>
./server_bin --listen-ip 127.0.0.1 --listen-port 5000
<br>
./proxy_bin --listen-ip 127.0.0.1 --listen-port 4000 --target-ip 127.0.0.1 --target-port 5000 --client-drop 0 --server-drop 0 --client-delay 0 --server-delay 0 --client-delay-time-min 100 --client-delay-time-max 300 --server-delay-time-min 150 --server-delay-time-max 300

###### 50% Drop, 0% Delay
./client_bin --target-ip 127.0.0.1 --target-port 4000 --timeout 3 --max-retries 50
<br>
./server_bin --listen-ip 127.0.0.1 --listen-port 5000
<br>
./proxy_bin --listen-ip 127.0.0.1 --listen-port 4000 --target-ip 127.0.0.1 --target-port 5000 --client-drop 50 --server-drop 50 --client-delay 0 --server-delay 0 --client-delay-time-min 100 --client-delay-time-max 300 --server-delay-time-min 150 --server-delay-time-max 300

###### 100% Drop, 0% Delay
./client_bin --target-ip 127.0.0.1 --target-port 4000 --timeout 3 --max-retries 1
<br>
./server_bin --listen-ip 127.0.0.1 --listen-port 5000
<br>
./proxy_bin --listen-ip 127.0.0.1 --listen-port 4000 --target-ip 127.0.0.1 --target-port 5000 --client-drop 100 --server-drop 100 --client-delay 0 --server-delay 0 --client-delay-time-min 100 --client-delay-time-max 300 --server-delay-time-min 150 --server-delay-time-max 300

###### 0% Drop, 50% Delay, 100-500ms
./client_bin --target-ip 127.0.0.1 --target-port 4000 --timeout 3 --max-retries 50
<br>
./server_bin --listen-ip 127.0.0.1 --listen-port 5000
<br>
./proxy_bin --listen-ip 127.0.0.1 --listen-port 4000 --target-ip 127.0.0.1 --target-port 5000 --client-drop 0 --server-drop 0 --client-delay 50 --server-delay 50 --client-delay-time-min 100 --client-delay-time-max 500 --server-delay-time-min 100 --server-delay-time-max 500

###### 0% Drop, 50% Delay, >= client timeout
./client_bin --target-ip 127.0.0.1 --target-port 4000 --timeout 1 --max-retries 50
<br>
./server_bin --listen-ip 127.0.0.1 --listen-port 5000
<br>
./proxy_bin --listen-ip 127.0.0.1 --listen-port 4000 --target-ip 127.0.0.1 --target-port 5000 --client-drop 0 --server-drop 0 --client-delay 50 --server-delay 50 --client-delay-time-min 1000 --client-delay-time-max 2000 --server-delay-time-min 1000 --server-delay-time-max 2000

