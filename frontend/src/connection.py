'''
Created on Feb 8, 2012

@author: blizzara
'''

import socket   # use BSD sockets
import struct   # used to convert to and fro binary data
import threading # each connection is a thread
import dmcr_protocol_pb2 as proto # import protobuf classes

# from socket.cpp
#  enum PacketId { Packet_BackendHandshake = 1, Packet_ConnectionResult = 2,
#                  Packet_NewTask = 3, Packet_RenderedData = 4 };



class Connection(threading.Thread):
    '''
    classdocs
    '''


    def __init__(self, conn, addr):
        '''
        Initialize and connect
        '''
        super(Connection, self).__init__()
        self.conn = conn
        self.addr = addr
        
        
    def __str__(self):
        '''
        
        @return: string version of a connection
        
        '''
        return "Socket connected to {0}".format(self.addr)
    
    def Close(self):
        '''
        Close connection
        
        '''
        self.conn.close()
    
    def run(self): # overriding threading.Thread.run()
    
        while True:
            self.ReceivePacket()
    
    def ReceiveHeader(self):
        '''
        Receives info about coming packet (id) and it's length.
        
        @return: a tuple containing (id, length)
        
        '''
        
        
        # receive first header length
        data = '' # empty string to store what we get
        while len(data) < 4: # first coming is 4 bytes
            data += self.conn.recv(4-len(data)) # recv may not give everything at once, so wait until we have 4 bytes
        
        data_tuple = struct.unpack("!L", data) # unpack binary data to long int (! means network)
        header_length  = int(data_tuple[0]) # int conversion maybe not needed, but first element of tuple anyways
        
        # if header length is usable, receive whole header
        if header_length < 1:
            return False
        
        header_data = self.ReceiveData(header_length)
        
        header = proto.PacketHeader()
        header.ParseFromString(header_data)
        
        return (header.id, header.length)
        
    def ReceiveData(self, length):
        data = ''
        data_length = 0
        while data_length < length:
            data += self.conn.recv(length-data_length)
            data_length = len(data)
        return data
        
    def ReceivePacket(self):
        packet_id, length = self.ReceiveHeader()
        
        data = self.ReceiveData(length)
        
        if packet_id == 1:
            self.Recv_BackendHandshake(data)
        elif packet_id == 2:
            self.Recv_ConnectionResult(data)
        elif packet_id == 3:
            self.Recv_NewTask(data)
        elif packet_id == 4:
            self.Recv_RenderedData(data)
        else:
            pass
        
    def Recv_BackendHandshake(self, data):
        
        handshake = proto.BackendHandshake()
        handshake.ParseFromString(data)
        
        self.protocol_version = handshake.protocol_version
        self.description = handshake.description
        print handshake # nothing clever to do yet, just print
        return handshake
        
    def Recv_ConnectionResult(self, data):
        conn_result = proto.ConnectionResult()
        conn_result.ParseFromString(data)
        result = conn_result.result
        print conn_result # nothing clever to do yet, just print
        return conn_result
    
    def Recv_NewTask(self, data):
        newtask = proto.NewTask()
        newtask.ParseFromString(data)
        
        print newtask # nothing clever to do yet, just print
        return newtask
        
    def RenderedData(self, data):
        rendered_data = proto.RenderedData()
        rendered_data.ParseFromString(data)
    
        print rendered_data # nothing clever to do yet, just print
        return rendered_data
