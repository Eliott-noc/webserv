import socket
import time
import sys

HOST = '127.0.0.1'
PORT = 8080

def test_basic_get():
    """Sends a standard GET request and verifies a response is returned."""
    print("\n--- TEST 1: Standard GET Request ---")
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((HOST, PORT))
        
        request = "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n"
        s.sendall(request.encode())
        
        response = s.recv(4096).decode()
        print("Response received from server:")
        print(response)
        s.close()
    except Exception as e:
        print(f"Error: {e}")

def test_segmented_request():
    """Sends a request in slow pieces to verify that the parsing loop
    correctly buffers partial segments (parse_status == 1) across poll() ticks."""
    print("\n--- TEST 2: Segmented (Partial) Request Buffer ---")
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((HOST, PORT))
        
        # Segment 1: Incomplete HTTP Headers
        part1 = "GET / HTTP/1.1\r\nHost: local"
        print(f"Sending segment 1: {repr(part1)}")
        s.sendall(part1.encode())
        
        # Sleep for a second to force poll() to process a partial read
        time.sleep(1.0)
        
        # Segment 2: Remaining headers and delimiters
        part2 = "host\r\nConnection: close\r\n\r\n"
        print(f"Sending segment 2: {repr(part2)}")
        s.sendall(part2.encode())
        
        response = s.recv(4096).decode()
        print("Response received from server:")
        print(response)
        s.close()
    except Exception as e:
        print(f"Error: {e}")

def test_multiple_concurrent_clients():
    """Opens several connections at the same time to ensure the poll() loop is non-blocking."""
    print("\n--- TEST 3: Concurrent Client Handshake ---")
    try:
        sockets = []
        num_clients = 3
        
        # 1. Establish connections
        for i in range(num_clients):
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            s.connect((HOST, PORT))
            sockets.append(s)
            print(f"Client {i+1} successfully connected. Keep connection open...")
            time.sleep(0.2)
            
        # 2. Send requests sequentially through the open connections
        for i, s in enumerate(sockets):
            request = "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n"
            s.sendall(request.encode())
            print(f"Sent request on Client {i+1}.")
            
        # 3. Read back the responses
        for i, s in enumerate(sockets):
            response = s.recv(1024).decode().split('\n')[0] # Get the status line (e.g., HTTP/1.1 200 OK)
            print(f"Client {i+1} received response status line: {response}")
            s.close()
            
    except Exception as e:
        print(f"Error: {e}")

if __name__ == '__main__':
    print("Checking if test environment is ready...")
    test_basic_get()
    time.sleep(1)
    test_segmented_request()
    time.sleep(1)
    test_multiple_concurrent_clients()