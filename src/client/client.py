import socket
import threading

data = b'A' * 100_000_000
sock = socket.socket()
sock.connect(('127.0.0.1', 8080))

received = 0
def reader():
    global received
    while received < len(data):
        buf = sock.recv(4096)
        if not buf:
            break
        received += len(buf)
    print("recv:", received)

t = threading.Thread(target=reader)
t.start()

total = 0
while total < len(data):
    n = sock.send(data[total:])
    total += n

t.join()
sock.close()
print("send:", total)
