import socket

TARGET_HOST = "127.0.0.1"
TARGET_PORT = 8080

def send_raw_request(name, raw_request, expected_status_prefixes):
    print(f"--- Running: {name} ---")
    try:
        # Using a raw socket prevents Python from auto-correcting malformed HTTP
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(2.0)
        s.connect((TARGET_HOST, TARGET_PORT))
        s.sendall(raw_request.encode('utf-8'))
        
        # Read the raw response
        response = s.recv(4096).decode('utf-8', errors='ignore')
        s.close()
        
        status_line = response.split('\r\n')[0] if response else "No Response"
        print(f"Status Line: {status_line}")
        
        if status_line.startswith(expected_status_prefixes):
            print("✅ PASS")
        else:
            print(f"❌ FAIL (Expected one of {expected_status_prefixes})")
    except Exception as e:
        print(f"❌ ERROR: {e}")
    print()

if __name__ == "__main__":
    # 1. Missing Host Header (HTTP/1.1 strictly requires it -> should return 400)
    req1 = "GET / HTTP/1.1\r\n\r\n"
    send_raw_request("Missing Host Header", req1, ("HTTP/1.1 400",))

    # 2. Path Traversal Attempt (Ensures the server prevents breaking out of the root directory)
    req2 = "GET /../../../../etc/passwd HTTP/1.1\r\nHost: localhost\r\n\r\n"
    send_raw_request("Path Traversal", req2, ("HTTP/1.1 403", "HTTP/1.1 404", "HTTP/1.1 400"))

    # 3. Invalid HTTP Method (Testing the parsing logic)
    req3 = "INVALIDMETHOD / HTTP/1.1\r\nHost: localhost\r\n\r\n"
    send_raw_request("Unknown Method", req3, ("HTTP/1.1 501", "HTTP/1.1 405", "HTTP/1.1 400"))

    # 4. Massive URI (Testing buffer limits and preventing memory overflows)
    huge_path = "/" + ("A" * 8000)
    req4 = f"GET {huge_path} HTTP/1.1\r\nHost: localhost\r\n\r\n"
    send_raw_request("Huge URI", req4, ("HTTP/1.1 414", "HTTP/1.1 400"))