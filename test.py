import socket

server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.bind(('127.0.0.1', 80))
server_socket.listen(1)

print("Listening on port 80...")

while True:
    client_socket, addr = server_socket.accept()
    print(f"Connection from {addr}")
    client_socket.sendall(b"HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n")
    client_socket.close()

