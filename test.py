import socket
import time

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(("127.0.0.1", 8080))

request = "POST /uploads/torture.txt HTTP/1.1\r\nHost: localhost\r\nContent-Length: 10\r\n\r\n"
for char in request:
    s.send(char.encode())
    print(f"Envoi de : {char}")
    time.sleep(0.5) # On attend une demi-seconde entre chaque lettre !

s.send("0123456789".encode())
print("Fini !")
s.close()