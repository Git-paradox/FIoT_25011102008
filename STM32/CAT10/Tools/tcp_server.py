import socket

HOST = "0.0.0.0"
PORT = 5000

print("CAT10 TCP sensor receiver")
print("Listening on port", PORT)

try:
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server:
        server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        server.bind((HOST, PORT))
        server.listen(1)

        while True:
            connection, address = server.accept()
            print("\nConnected:", address)

            with connection:
                while True:
                    data = connection.recv(1024)
                    if not data:
                        break

                    print(data.decode("utf-8", errors="replace"), end="", flush=True)

            print("\nConnection closed")

except KeyboardInterrupt:
    print("\nServer stopped")
