"""A simple script to send some synthetic values to the MeterReceiver server to test it."""

import CounterUpdate_pb2
import random
import socket
import struct
import time
import binascii

TCP_IP = '127.0.0.1'
TCP_PORT = 2110
MAGIC_STRING = 'AA'

def send_message(sock, message):
    """ Send a serialized message (protobuf Message interface)
        to a socket, prepended by a magic header, the length of
        the message packed in 4 bytes (big endian), followed by 
        the crc32 value of the protobuf message in 4 bytes (big endian).
    """
    s = message.SerializeToString()
    packed_len = struct.pack('>L', len(s))

    crc = struct.pack('>L', (binascii.crc32(s) & 0xFFFFFFFF))

    sock.sendall(MAGIC_STRING + packed_len + s + crc)

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((TCP_IP, TCP_PORT))
print "Connected"

def send_garbage(sock, amount):
    for i in range(amount):
        sock.send(chr(random.randint(0, 255)))


series=0
count=0
while True:
    inc = CounterUpdate_pb2.CounterUpdate()

    inc.meterId = 1
    if random.randint(0,10) == 1:
        # new series
        series += 1
        count = 0

    count += random.randint(0, 10)

    inc.seriesId = series
    inc.currentCounterValue = count

    send_message(s, inc)
    if random.randint(0, 10) < 3:
        send_garbage(s, random.randint(1, 50))

    time.sleep(1)



s.close()

