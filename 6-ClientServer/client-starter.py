import socket  # Import socket module for network communication

# Create a socket object for the client (TCP)
ClientSocket = socket.socket()

# Define the server's host and port
host = '127.0.0.1'  # Localhost (server is running on the same machine)
port = 1233         # Must match the server port

print('Waiting for connection')

try:
    # Attempt to connect to the server
    ClientSocket.connect((host, port))
except socket.error as e:
    print(str(e))

# Receive the initial welcome message from the server
Response = ClientSocket.recv(1024)
print(Response.decode('utf-8'))

# Loop for user interaction
while True:
    Input = input('Say Something (type "exit" to disconnect): ')
    
    if Input.lower() == 'exit':
        print('Disconnecting from server...')
        break  # Exit the loop and close connection
    
    ClientSocket.send(str.encode(Input))        # Send user input to server
    Response = ClientSocket.recv(1024)          # Receive response
    print(Response.decode('utf-8'))             # Print decoded server response

# Close the socket after exiting the loop
ClientSocket.close()
