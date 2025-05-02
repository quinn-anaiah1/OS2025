import socket
import os
from _thread import *  # For handling multiple clients via threads

# Create a TCP/IP socket
ServerSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Define the server address
host = '127.0.0.1'  # Localhost
port = 1233
ThreadCount = 0

try:
    # Bind the socket to the address
    ServerSocket.bind((host, port))
except socket.error as e:
    print(str(e))

print('Waiting for a Connection ..')

# Start listening for incoming connections (up to 5 queued)
ServerSocket.listen(5)

# Function to handle a single client's communication
def threaded_client(connection):
    connection.send(str.encode('Welcome to the server'))  # Send initial message
    while True:
        data = connection.recv(2048)  # Wait for client message
        if not data:
            break  # Connection was closed
        
        message = data.decode('utf-8')
        if message.lower() == 'exit':
            print('Client requested to disconnect.')
            break  # Exit the loop on 'exit' command

        reply = 'Server says ' + message
        connection.sendall(str.encode(reply))  # Echo back with prefix

    connection.close()  # Close the connection
    print('Connection closed.')

# Main loop to accept and handle incoming connections
while True:
    Client, address = ServerSocket.accept()
    print('Connected to: ' + address[0] + ':' + str(address[1]))
    start_new_thread(threaded_client, (Client,))
    ThreadCount += 1
    print('Thread Number: ' + str(ThreadCount))

# Will not be reached, but in case of future change
ServerSocket.close()


''' Heres the server py'''
'''
from socket import *
serverPort =1200
serverSocket = socket(AF_INET, SOCK_STREAM)
servetSocket.bind(('', serverPort))
serverSocket.listen(1)
print 'The servers is ready to recieve'
while True:
    connectionSocket, addr = serverSocket.accept()
    sentence = connectionSocket.recv(1024).decode
    capitalizedSentence = sentence.upper()
    connectionSocket.send(capitalizedSentence.encode())
    connectionSocket.close()

'''
#CLient py
'''
from socket import *
serverName = 'servername'
serverPort =1200
clientSocket = socket(AF_INET, SOCK_STREAM)
clientSocket.connect((serverName, serverPort))
sentence = raw_input('Input lowercase sentence:')
clientSocket.send(sentence.encode())
modifiedSentence = clientSocker.recv(1024)
print('From Server:', modifiedSentence.decode())
clientSocket.close()

'''