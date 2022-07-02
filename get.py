import socket

r = b""

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect(("192.168.0.3", 1234))
    s.sendall(b"o")
    while r.count(b"\n") < 2:
        r += s.recv(1024)

print(repr(r))
