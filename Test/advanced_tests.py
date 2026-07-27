import socket
import time

TARGET_HOST = "127.0.0.1"
TARGET_PORT = 8080

def test_pipelining():
    print("--- Running: HTTP Pipelining (Double Request) ---")
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(2.0)
        s.connect((TARGET_HOST, TARGET_PORT))
        
		# Sending two complete GET requests with keep-alive specified
        payload = (
            "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: Keep-Alive\r\n\r\n"
            "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: Keep-Alive\r\n\r\n"
        )
        s.sendall(payload.encode('utf-8'))
        s.sendall(payload.encode('utf-8'))
        
        # Read the response
        response = s.recv(8192).decode('utf-8', errors='ignore')
        s.close()
        
        # Count how many times the server responded with an HTTP status line
        status_count = response.count("HTTP/1.1 ")
        
        if status_count == 2:
            print("✅ PASS: Server responded to both requests in the pipeline.")
        elif status_count == 1:
            print("❌ FAIL: Server only responded to the first request. It likely cleared the read buffer too early.")
        else:
            print(f"❌ FAIL: Unexpected response count. Found {status_count} status lines.")
            
    except Exception as e:
        print(f"❌ ERROR: {e}")
    print()

def test_slowloris():
    print("--- Running: Slowloris (Partial Headers) ---")
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((TARGET_HOST, TARGET_PORT))
        
        # Send the start of a request, but leave off the final \r\n\r\n
        print("  -> Sending partial request...")
        s.sendall(b"GET / HTTP/1.1\r\nHost: localhost\r\n")
        
        # Wait for 5 seconds. A blocking server will freeze here.
        print("  -> Waiting 5 seconds. If your server is blocking, no other clients can connect right now.")
        time.sleep(5)
        
        # Send the rest of the request
        print("  -> Sending the rest of the request...")
        s.sendall(b"X-Test-Header: Done\r\n\r\n")
        
        response = s.recv(4096).decode('utf-8', errors='ignore')
        s.close()
        
        if response.startswith("HTTP/1.1"):
            print("✅ PASS: Server kept the connection alive, didn't block, and eventually responded.")
        else:
            print("❌ FAIL: Server returned an invalid response.")
            
    except ConnectionResetError:
        print("✅ PASS (Timeout): Server forcefully dropped the stagnant connection (this is a valid defense!).")
    except Exception as e:
        print(f"❌ ERROR: {e}")
    print()

def test_premature_disconnect():
    print("--- Running: Premature Disconnect (Ghost Client) ---")
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((TARGET_HOST, TARGET_PORT))
        
        # Send a mangled, incomplete request
        s.sendall(b"GET / HTT")
        
        # Instantly brutally close the socket from the client side
        s.close()
        
        print("✅ Client disconnected mid-request.")
        print("⚠️ ACTION REQUIRED: Check your server's terminal! If it crashed (Segmentation fault or SIGPIPE), this test FAILED. If it is still running, this test PASSED.")
        
    except Exception as e:
        print(f"❌ ERROR connecting to server: {e}")
    print()

if __name__ == "__main__":
    test_pipelining()
    test_slowloris()
    test_premature_disconnect()