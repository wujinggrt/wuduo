#!/usr/bin/python3

import socket
import threading
import sys

def client_request(num :int):
    server_ip = '127.0.0.1'
    server_port = 12000
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect((server_ip, server_port))

    # sentence = input('Input lowercase sentence:')
    sentence = "Hello server, I am " + str(num)
    recved = ''

    client_socket.send(sentence.encode())

    modified = client_socket.recv(1024)
    recved = '[{}] From server:{}'.format(num, modified.decode())
    '''
    except:
        recved = '[{}] Failed to deal socket fd'.format(num)
    '''

    print('[{}]'.format(num), 'local:', client_socket.getsockname())
    print(recved)

    client_socket.close()

if __name__ == '__main__':
    assert len(sys.argv) == 2 
    threads = []
    for i in range(int(sys.argv[1])):
        threads.append(threading.Thread(target=client_request, args=(i,)))
        threads[i].start()

    for t in threads:
        t.join();
    

