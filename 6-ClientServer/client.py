import socket

# Server configuration
server_host = '127.0.0.1'
server_port = 1200
chunk_size = 1000

# Create a TCP socket
clientSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
clientSocket.connect((server_host, server_port))
print(f"Connected to server at {server_host}:{server_port}")

# Receive the client ID as a short message
client_id = clientSocket.recv(16).decode().strip()
output_filename = f"received_bio_{client_id}.txt"

# Write incoming file data to the output file
try:
    with open(output_filename, 'wb') as f:
        while True:
            data = clientSocket.recv(chunk_size)
            if not data:
                break
            f.write(data)

    print(f"File received and saved as {output_filename}")
except Exception as e:
    print(f"!Error: {e}")
finally:
    clientSocket.close()
