import socket
from _thread import start_new_thread

# Server settings
host = '127.0.0.1'   # Localhost
port = 1200          # Port to bind the server
chunk_size = 1000    # Send file in 1000-byte chunks
filename = "bio.txt"  # File to be transferred


# Create TCP socket
serverSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

try:
    # Bind the socket to host and port
    serverSocket.bind((host, port))
except socket.error as e:
    print(f"Bind failed: {e}")
    exit()

# Start listening for incoming connections
serverSocket.listen(5)
print(f"Server is listening on {host}:{port}")

# Thread function to handle file transfer for each client
def threaded_client(connection, client_id):
    print(f"Client {client_id} connected")

    try:
        # Send client ID as a short text message
        connection.sendall(f"{client_id}".encode())
        with open(filename, "rb") as f:
            while True:
                # Read 1000 bytes at a time
                data = f.read(chunk_size)
                if not data:
                    break
                # Send the data chunk to client
                connection.sendall(data)

        print(f"File sent to client {client_id}")
    except Exception as e:
        print(f" !Error with client {client_id}: {e}")
    finally:
        connection.close()

# Keep track of connected clients
client_id = 0
serverSocket.settimeout(1.0)  # Timeout every 1 second tp check for interrupts.

# Main loop to accept multiple clients
try:
    while True:
        try:
            client_socket, address = serverSocket.accept()
            client_id += 1
            print(f"Connection from {address[0]}:{address[1]}")
            # Start a new thread for each client
            start_new_thread(threaded_client, (client_socket, client_id))
        except socket.timeout:
            pass # Loop again, allows checking for Ctrl+
except KeyboardInterrupt:
    print("\n Server shut down  requested")
finally:
    serverSocket.close()
    print('Server socket closed')
