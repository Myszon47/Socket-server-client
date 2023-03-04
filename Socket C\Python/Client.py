import socket

if __name__ == "__main__":
    HOST = input("Type a host name: \n") 
    PORT = int(input("Type a port number: \n"))
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((HOST, PORT))
        filePath = input("Type file path of file you want get: \n")
        s.send(filePath.encode())
        with s,open(f"/home/students/s473575/Desktop/siecitestodbioru/test",'wb') as file:
            while True:
                recvfile = s.recv(8192)
                if not recvfile: break
                file.write(recvfile)
        print("File has been received.")
