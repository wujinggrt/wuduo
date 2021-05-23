#/usr/bin/python3

import socket

server_ip = '127.0.0.1'
server_port = 12000
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client_socket.connect((server_ip, server_port))

sentence = input('Input lowercase sentence:')
client_socket.send(sentence.encode())

modified = client_socket.recv(1024)
print('From server: ', modified.decode())

client_socket.close()
