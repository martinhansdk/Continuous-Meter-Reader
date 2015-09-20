import MeterReader_pb2
import struct
import time
import binascii

MAX_PROTOBUF_MSG_LEN = 150
MAGIC_STRING = 'AA'

def send_message(conn, message):
    """ Send a serialized message (protobuf Message interface)
        to a connet in the following format:
          * the magic header
          * the length of the encoded protobuf message in 4 bytes (little endian)
          * the encoded protobuf message
          * the crc32 value of the protobuf message in 4 bytes (little endian)
    """
    s = message.SerializeToString()
    packed_len = struct.pack('<L', len(s))

    crc = struct.pack('<L', (binascii.crc32(s) & 0xFFFFFFFF))

    coded_msg = MAGIC_STRING + packed_len + s + crc
    conn.write(coded_msg)
    #conn.sendall(coded_msg)

def receive_message(conn):
    """ Receive one serialized message and return it, discarding any
    malformed data."""

    while True:
      # Find the first occurrence of the magic string which is AA
      seen = 0
      garbage = 0
      while seen != 2:
          c = conn.read()
          print ord(c)

          if ord(c) == 65:
              seen += 1
          else:
              seen = 0
              garbage += 1
        
      if garbage > 0:
          print "Discarded %d bytes of garbage" % garbage
      

      # Read the length field
      length = struct.unpack('<L', conn.read(4))[0]

      print "len=", length

      if length > MAX_PROTOBUF_MSG_LEN:
          print "Message length unrealistically large. Skipping. len=%s" % len
          continue

      # Create a data buffer of type byte slice with capacity for the message
      data = conn.read(length)

      # Read the checksum and match it against the received data
      crc = (binascii.crc32(data) & 0xFFFFFFFF)
      expectedCRC = struct.unpack('<L', conn.read(4))[0]

      if crc != expectedCRC:
        print "Checksum mismatch, skipping. Header says 0x%08x, calculated 0x%08x\n" % (expectedCRC, crc)
        continue
      
      message=MeterReader_pb2.Message()
      message.ParseFromString(data)
      return message

