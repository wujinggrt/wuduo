#!/usr/bin/python3

import socket
import re

def respond(client_socket):
    print('connection established, recving request_data')
    request_data = client_socket.recv(1024 * 1024)
    print('recved request_data')
    uri = ''
    if request_data:
        request_str = request_data.decode()
        print('请求头：\n',request_str)
        # [^/]+：匹配任意字符串一直到/停， 只要不是/，前面至少有一个， ^在[]中为取反，不在[]为匹配字符串开头
        # [^/]+/(.*?)\sHTTP/1.1
        # [ ^] *: 一直匹配到空格停
        ret = re.match(r'[^/]+(/[^ ]*)', request_str)
        uri = ret.group(1)
        if uri == '/':
            uri = '/index.html'
 
    print('请求的资源：',uri)
    try:
        # 发送响应http格式的信息给浏览器
        header = 'HTTP/1.1 200 OK\n '
        header += 'Content-Type: text/html;charset=utf-8 \r\n'
        header += '\n'
        # 用h1 html标签浏览器可以立即加载出信息，用纯文本发送数据，浏览器会认为你还有数据没发送完，一直加载，不显示信息
        # 读取html文件夹中的index.html文件，作为响应的body
        file = open('test.html', 'rb')
        body = file.read()
        file.close()
        print('响应体：\r\n',body)
        # Can't convert 'bytes' object to str implicitly
        # response_data += body
        # 可以将response分部发送给浏览器，这个整个程序执行完一次才叫一次服务器的响应过程
        # 将response header部分发送给浏览器
        client_socket.send(header.encode())  # encode()为编码，编码成字节码，io中可传输的数据格式
        # 将response body部分发送给浏览器
        client_socket.send(body)
    except:
        header = 'HTTP/1.1 404 NOT FOUND\r\n '
        header += 'Content-Type: text/html;charset=utf-8 \r\n'
        header += '\r\n'
        body = '<h1>NOT FOUND</h1>'
        # 将response header部分发送给浏览器
        client_socket.send(header.encode())  # encode()为编码，编码成字节码，io中可传输的数据格式
        # 将response body部分发送给浏览器
        client_socket.send(body.encode())
    client_socket.close()


server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
server_socket.bind(('0.0.0.0', 8080))
server_socket.listen(128)
num_client_visited = 0
print("Server started on {}".format(8080))
while True:
    client_socket, client_addr = server_socket.accept()
    num_client_visited += 1
    print("num_client_visited {}".format(num_client_visited))
    respond(client_socket)

server_socket.close()
