#!/usr/bin/python3

import socket
import threading
import sys

error = False

bad_request = 'ET / HTTP/1.1\r\n\r\n'
correct_sentence = 'GET / HTTP/1.1\r\nClient: python\r\n\r\n'
#correct_sentence = bad_request

def client_request(num, sentence):
    if error:
        return None
    server_ip = '127.0.0.1'
    server_port = 12000
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect((server_ip, server_port))

    # sentence = input('Input lowercase sentence:')
    # sentence = "Hello server, I am " + str(num)
    recved = ''

    client_socket.send(sentence.encode())

    modified = 'nothing'.encode()
    try:
        modified = client_socket.recv(1024)
        recved = '[{}] From server:\n{}'.format(num, modified.decode())
    except:
        recved = '[{}] Failed to deal socket fd, modified[{}]'.format(num, modified.decode())
        error

    print('[{}]'.format(num), 'local:', client_socket.getsockname())
    print(recved)

    client_socket.close()

if __name__ == '__main__':
    assert len(sys.argv) == 2 or len(sys.argv) == 3
    if len(sys.argv) == 3:
        correct_sentence = bad_request
    threads = []
    num_requests = int(sys.argv[1])
    if num_requests > 5000:
        chunk = 5
        for j in range(num_requests // chunk):
            for i in range(chunk):
                threads.append(threading.Thread(target=client_request, args=(i + j * chunk, correct_sentence)))
                threads[i].start()

            for t in threads:
                t.join();
            
            threads = []
    else:
        for i in range(int(sys.argv[1])):
            threads.append(threading.Thread(target=client_request, args=(i, correct_sentence)))
            threads[i].start()

        for t in threads:
            t.join();
