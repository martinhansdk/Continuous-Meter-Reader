"""A simple script to send some synthetic values to the MeterReceiver server to test it."""

import Message

import MeterReader_pb2
import random
import socket
import time


TCP_IP = '127.0.0.1'
TCP_PORT = 2110

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((TCP_IP, TCP_PORT))
print "Connected"

def send_garbage(sock, amount):
    for i in range(amount):
        sock.send(chr(random.randint(0, 255)))


series=0
count=0
while True:
    msg = MeterReader_pb2.Message()

    msg.update.meterId = 1
    if random.randint(0,10) == 1:
        # new series
        series += 1
        count = 0

    count += random.randint(0, 10)

    msg.update.seriesId = series
    msg.update.currentCounterValue = count

    Message.send_message(s, msg)
    if random.randint(0, 10) < 3:
        send_garbage(s, random.randint(1, 50))

    time.sleep(1)



s.close()

