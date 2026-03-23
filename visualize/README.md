## Installation
###### Install uv project manager

curl -LsSf https://astral.sh/uv/install.sh | sh

###### Setup uv project
uv init --bare
<br>
uv venv --python 3.14.3
<br>
source .venv/bin/activate
<br>
uv add matplotlib
<br>
uv add pyinstaller
<br>

###### Build binary (optional)
pyinstaller --onefile visualize_logs.py

###### Example run command using .py
python3 visualize_logs.py --client-log ../client.log --proxy-log ../proxy.log --server-log ../server.log --out results.png

###### Example run command using binary
./dist/visualize_logs --client-log ../client.log --proxy-log ../proxy.log --server-log ../server.log --out results.png

