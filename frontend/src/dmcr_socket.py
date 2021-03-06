'''
 This file is a part of the DMCR project and is subject to the terms and
 conditions defined in file 'LICENSE.txt', which is a part of this source
 code package.
'''

'''
Created on Mar 14, 2012

@author: blizzara
'''

import socket   # use BSD sockets
import struct   # used to convert to and fro binary data

import dmcr_protocol_pb2 as proto # import protobuf classes


# from socket.cpp
#  enum PacketId { Packet_BackendHandshake = 1, Packet_ConnectionResult = 2,
#                  Packet_NewTask = 3, Packet_RenderedData = 4 };


# enum ConnectionResult { ConnectionResult_Success, ConnectionResult_InvalidKey,
#                       ConnectionResult_ConnectionFailed };


class Socket(object):
    '''
    classdocs
    '''

    BACKENDHANDSHAKE = 1
    CONNECTIONRESULT = 2
    NEWTASK = 3
    RENDEREDDATA = 4
    QUITTASK = 5


    CONNECTIONRESULT_SUCCESS = 0
    CONNECTIONRESULT_INVALIDKEY = 1
    CONNECTIONRESULT_CONNECTIONFAILED = 2
    
    PNG_8BIT = 1
    PNG_16BIT_CLAMPED = 2

    class SocketException(Exception):
        def __init__(self, value = ""):
            self.value = value
            
        def __str__(self):
            return repr(self.value)
        


    def __init__(self,connection, address):
        '''
        Constructor
        '''
        
        self.conn = connection
        self.addr = address
        self.conn.settimeout(10)
        self.stopped = False
        self.alive = True 
    
    def Close(self):
        '''
        Closes connection
        
        @return: nothing
        
        '''
        if self.alive:
            self.conn.settimeout(1)
            self.conn.close()
            self.alive = False
    
    def ReceiveData(self, length):
        '''
        Receives data from self.conn. 
        Returns when has received asked number of bytes.
        Raises Socket.SocketException if someone calls Connection.stop() 
        while this is running.
        
        @param length: how many bytes are wanted (int)
        @return: received data (as it was received)
        
        '''
        data = ''
        data_length = 0
        recvd_data = ""
        while data_length < length:
            if self.stopped:
                raise Socket.SocketException("Stopped while receiveng data")
            try: 
                recvd_data = self.conn.recv(length-data_length)
                if not recvd_data:
                    self.alive = False
                    raise Socket.SocketException("Connection to client " + self.addr[0] + " closed")
                data += recvd_data
            except socket.timeout:
                pass
            data_length = len(data)
        return data
    
    def ReceiveHeader(self):
        '''
        Receives info about coming packet (id) and it's length.
       
        @return: a tuple containing (id, length)
        
        '''
        
        # receive first header length
        try:
            data = self.ReceiveData(4) # first comes 4 bytes indicating headers length
#        except SocketException as se:   # we might fail, but it doesn't matter
#            return False, False
        finally:
            pass
             
        data_tuple = struct.unpack("!L", data)
        # unpack binary data to long int (! means network)
        header_length  = int(data_tuple[0]) # int conversion maybe not needed, but first element of tuple anyways
        
        # if header length is usable, receive whole header
        if header_length < 1:
            return False, False
        
        try:
            header_data = self.ReceiveData(header_length)
#        except SocketExecption:
#            return False, False
        finally: #need something more than just try
            pass

        header = proto.PacketHeader()
        header.ParseFromString(header_data)
        
        return header.id, header.length
        
        
    def ReceivePacketData(self):
        id, length = self.ReceiveHeader()
        return id, self.ReceiveData(length)


    
    def Recv_BackendHandshake(self):
        '''
        Receives handshake from BE and returns a tuple containing (protocol_version, backend_description) or False if failed 
        
        
        @return: received BackendHandshake-protobuf packet, False if failed 
        
        ''' 

        packet_id, data = self.ReceivePacketData() # receive header and data
        if packet_id != Socket.BACKENDHANDSHAKE: # if packet wasn't what we wanted
            return False    # quit
        
        handshake = proto.BackendHandshake()
        handshake.ParseFromString(data)
        
        #self.protocol_version = handshake.protocol_version
        #self.description = handshake.description
        #print handshake # nothing clever to do yet, just print
        
        #self.Send_ConnectionResult(CONNECTIONRESULT_SUCCESS)
        #self.Send_NewTask(1,400,300,10,self.scene)
        return handshake.protocol_version, handshake.description


    def Recv_RenderedData(self):
        '''
        Receives rendered data from BE and returns it as a list of pixels or False if failed.

        
        Reads incoming image size from packet, then receives the image,
        converts it to list of pixels and writes to "test.ppm".
        
        @param data: binary data containing BackendHandshake-protobuf packet
        @return: a 1d list containing pixels of image as a list of rgb components 
        
        ''' 
        packet_id, data = self.ReceivePacketData() # receive header and data
        if packet_id != Socket.RENDEREDDATA: # if packet wasn't what we wanted
            return False    # quit

        rendered_data = proto.RenderedData()
        rendered_data.ParseFromString(data)
        data_len = rendered_data.data_length
        try:
            data = self.ReceiveData(data_len)
#        except SocketException: # here we might want to capture this, but safer if we don't..
#            print "Thread closed while receiving picture."
#            return False
        finally: # need something more than just try
            pass 

        #print rendered_data
        return {'task_id':rendered_data.id, 'data':data, 'iterations_done':rendered_data.iterations_done, 'data_format':rendered_data.data_format}
        
        #The old code for PPM
        '''
        i = 0
        pixels = list()
        while i + 11 < data_len:
            pixels.append([0,0,0,0])
            pixels[-1][0] = int(struct.unpack("!L", data[i:i+4])[0] / 16843009)
            pixels[-1][1] = int(struct.unpack("!L", data[i+4:i+8])[0] / 16843009)
            pixels[-1][2] = int(struct.unpack("!L", data[i+8:i+12])[0] / 16843009)
            
            #pixels.append(struct.unpack("!L"*3, data[i,i+12])) #doesn't work
            i = i + 12
        if len(pixels) != rendered_data.width * rendered_data.height:
            print "Dimension mismatch: got {} pixels, but image size should be {} x {}".format(len(pixels),rendered_data.width, rendered_data.height)
            return False
        return pixels
        '''
        
        
 


    def SendHeader(self, id, length):
        '''
        Sends info about coming packet, first the length of header and then
        the header (Protobuf-packet) itself: packet id and packet length.
        
        @return: nothing
        
        '''
        
        head = proto.PacketHeader()
        head.length = length
        head.id = id
        
        
        
        head_size = head.ByteSize()
        head_size_bin = struct.pack("!L", head_size) 
        
        
        self.SendData(head_size_bin) # send size of header as !L
        
        self.SendData(head.SerializeToString()) # send the header
                
    def SendData(self, data):
        ''' 
        Just sends given data to client.
        
        @param data: binary data to be sent (string)
        
        @return: nothing 
        
        '''
        self.conn.sendall(data)
        
    def SendPacket(self, type_id, protobuf_packet):
        '''
        Sends packet (data) of type (type_id). Handles sending of header and packet itself.
        
        @param type_id: type of packet (enum)
        @param protobuf_packet: packet to be sent as a protobuf-packet
        
        @return: nothing
        
        '''

        self.SendHeader(type_id, protobuf_packet.ByteSize())
        self.SendData(protobuf_packet.SerializeToString())
        
    # No need to send this packet from frontend
    '''
    def Send_BackendHandshake(self, data):
        
        handshake = proto.BackendHandshake()
        handshake.ParseFromString(data)
        
        self.protocol_version = handshake.protocol_version
        self.description = handshake.description
        print handshake # nothing clever to do yet, just print
        return handshake
    '''
         
    def Send_ConnectionResult(self, result):
        '''
        Creates ConnectionResult -protobuf packet and sends it.
        
        @param result: result to be sent (eg. CONNECTIONRESULT_SUCCESS)
        
        @return: nothing
        
        ''' 
        conn_result = proto.ConnectionResult()
        conn_result.result = result
        
        self.SendPacket(Socket.CONNECTIONRESULT, conn_result)
        
    def Send_NewTask(self, task_id, width, height, iterations, scene):
        '''
        Creates NewTask -protobuf packet and sends it.
        
        @param task_id: id of task to be sent (int)
        @param width: wanted width of image (int)
        @param height: wanted height of image (int)
        @param iterations: how many iterations we want to be done (int)   
        @param scene: the json scene-file to be rendered (string) 
        
        @return: nothing
        '''
        newtask = proto.NewTask()
        newtask.id = task_id
        newtask.width = width
        newtask.height = height
        newtask.iterations = iterations
        newtask.scene = scene
        
        self.SendPacket(Socket.NEWTASK, newtask)
    
    def Send_QuitTask(self, task_id):
        '''
        Tells backend to stop rendering this task.
        
        '''
        quittask = proto.QuitTask()
        quittask.id = task_id
        
        self.SendPacket(Socket.QUITTASK, quittask)


# these following packets are sent by frontend, no need to know how to receive them

    '''    def Recv_ConnectionResult(self, data):
        conn_result = proto.ConnectionResult()
        conn_result.ParseFromString(data)
        result = conn_result.result
        print conn_result # nothing clever to do yet, just print
        return conn_result
    '''
    
    '''    def Recv_NewTask(self, data):
        newtask = proto.NewTask()
        newtask.ParseFromString(data)
        
        print newtask # nothing clever to do yet, just print
        return newtask
    '''      
        
        #No need to send this packet from frontend
'''    def Send_RenderedData(self, data):
        rendered_data = proto.RenderedData()
        rendered_data.ParseFromString(data)
    
        print rendered_data # nothing clever to do yet, just print
        return rendered_data
        '''
