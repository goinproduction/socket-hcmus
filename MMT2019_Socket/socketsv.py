import socket


def createSocketServer():
    HOST = '127.0.0.1'
    PORT = 8080

    # Tien hanh tao socket server dung thu vien socket
    socketServer = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
    socketServer.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR,1)
    socketServer.bind((HOST,PORT))
    socketServer.listen(1)

    print('Serving on port ',PORT) 
    while True:
        (connection, address) = socketServer.accept()
        request = connection.recv(1024).decode('utf-8')
        string_list = request.split(' ')
        method = string_list[0]
        requestingFile = string_list[1] # Lay ten file tu thanh dia chi(Ex: localhost:8080/index.html -> requestingFile = 'index.html')
    
        print('Client file requesting: ',requestingFile) # In rq_f ra man hinh console
    
        fileRecv = requestingFile.split('?')[0]
        fileRecv = fileRecv.lstrip('/') # Localhost:8080/ -> Trang mac dinh duoc mo se la index.html
        if(fileRecv == ''):
            fileRecv = 'index.html'

        # Tien hanh doc file request tu phia client
        try:
            file = open(fileRecv,'rb')
            response = file.read()
            file.close()
    
            header = 'HTTP/1.1 200 OK\n'
    
            if(fileRecv.endswith(".jpg")):
                mimetype = 'image/jpg'
            elif(fileRecv.endswith(".css")):
                mimetype = 'text/css'
            elif(fileRecv.endswith(".js")):
                mimetype = 'text/js'    
            else:
                mimetype = 'text/html'
    
            header += 'Content-Type: '+str(mimetype)+'\n\n'
        
        # Bat exceptopn neu dung dung nhap sai ten file(Ex: localhost:8080/if.html, vi file if.htm; k ton tai -> tra ve 404)
        except Exception as e:
            header = 'HTTP/1.1 404 Not Found\n\n'
            response = '<html><body><center><h3>Error 404: File not found</h3><p>Python HTTP Server</p></center></body></html>'.encode('utf-8')
 
        finalResponse = header.encode('utf-8') # Tien hanh encode header
        finalResponse += response
        connection.send(finalResponse) # Tra ve response cho client
        connection.close()
createSocketServer()
 
