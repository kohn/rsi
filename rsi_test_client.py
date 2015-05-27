#! /usr/bin/python
# -*- coding: utf-8 -*-
#############################################
# Copyright (C) 2010-2015
#
# filename: rsi_test_client.py
# date: Wed May 27 13:55:26 2015
# author: Kohn<kohntzx@gmail.com>
# comment: used to test rsi_server
########################################
import socket
import json
import signal
sock = None
def get_socket(ip_address="10.214.144.184", port=7209):
    print "connecting..."
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.settimeout(5)
    try:
        sock.connect((ip_address, port))
        print "connect to " + ip_address
    except:
        print "could not connect to " + ip_address
    return sock

def communicate(sock, msg):
    sock.send(msg)
    print "Response for " + msg
    print sock.recv(4096)

def my_signal_handler(a, b):
    print 'SIGINT captured'
    print "disconnecting"
    sock.close()
    exit()
    
if __name__ == '__main__':
    signal.signal(signal.SIGINT, my_signal_handler)
    sock = get_socket()
    communicate(sock, "GET_HOST_MEM_USAGE")
    communicate(sock, "GET_HOST_NODE_INFO")
    communicate(sock, "GET_VM_INFO")
    communicate(sock, "GET_VM_DETAIL 22")
    communicate(sock, "GET_HOST_CPU_USAGE")
    while True:
        data = raw_input("input data to be sent:")
        communicate(sock, data)
