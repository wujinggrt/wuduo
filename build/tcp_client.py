#!/usr/bin/python3

import socket
import threading

def client_request():
    server_ip = '127.0.0.1'
    server_port = 12000
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect((server_ip, server_port))

    # sentence = input('Input lowercase sentence:')
    sentence = "Hello server"
    client_socket.send(sentence.encode())

    modified = client_socket.recv(1024)
    print('From server: ', modified.decode())

    client_socket.close()

if __name__ == '__main__':
    threads = []
    for i in range(10):
        threads.append(threading.Thread(target=client_request))
        threads[i].start()

    for t in threads:
        t.join();
    

