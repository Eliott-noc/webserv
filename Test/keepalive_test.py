import socket
import time
import re

def recv_full_response(sock, timeout=5.0):
    sock.settimeout(timeout)
    response_data = b""
    
    # 1. Read byte-by-byte or in chunks until the end of headers (\r\n\r\n)
    while b"\r\n\r\n" not in response_data:
        try:
            chunk = sock.recv(4096)
            if not chunk:
                break # Connection closed by server
            response_data += chunk
        except socket.timeout:
            print("[-] Timeout while reading headers")
            return None, b""
            
    if not response_data:
        return None, b""

    # Split headers and the initial part of the body
    headers_raw, body_raw = response_data.split(b"\r\n\r\n", 1)
    headers = headers_raw.decode('utf-8', errors='ignore')
    
    # 2. Extract Content-Length using regex
    content_length = 0
    match = re.search(r'(?i)Content-Length:\s*(\d+)', headers)
    if match:
        content_length = int(match.group(1))
        
    # 3. Keep reading until we have the full body
    body = body_raw
    while len(body) < content_length:
        try:
            chunk = sock.recv(4096)
            if not chunk:
                break # Connection closed prematurely
            body += chunk
        except socket.timeout:
            print("[-] Timeout while reading body")
            break
            
    return headers, body

def test_keep_alive(host="127.0.0.1", port=8080):
    print(f"--- Testing Keep-Alive on {host}:{port} ---")
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((host, port))
        
        # --- Request 1 ---
        req1 = f"GET / HTTP/1.1\r\nHost: {host}\r\nConnection: keep-alive\r\n\r\n"
        print("\n[1] Sending Request 1 (Connection: keep-alive)...")
        s.sendall(req1.encode())
        
        headers1, body1 = recv_full_response(s)
        if not headers1:
            print("\n[FAIL] Server returned empty response and closed the fd immediately.")
            s.close()
            return
            
        print("--- Received Response 1 Headers ---")
        print(headers1)
        print(f"--- Body Length Read: {len(body1)} bytes ---")
        
        print("\nWaiting 1 second before sending Request 2 on the EXACT SAME fd...")
        time.sleep(1)
        
        # --- Request 2 ---
        req2 = f"GET / HTTP/1.1\r\nHost: {host}\r\nConnection: close\r\n\r\n"
        print("\n[2] Sending Request 2 (Connection: close)...")
        s.sendall(req2.encode())
        
        headers2, body2 = recv_full_response(s)
        
        # Check if we actually got a valid HTTP response for the second request
        if headers2 and "HTTP/" in headers2:
            print("--- Received Response 2 Headers ---")
            print(headers2)
            print(f"--- Body Length Read: {len(body2)} bytes ---")
            print("\n[SUCCESS] Server successfully handled multiple requests on the same connection!")
        else:
            print("\n[FAIL] Server did not return a valid HTTP response for the second request.")
            print("It likely closed the connection after the first response.")
            
        s.close()
        
    except ConnectionResetError:
        print("\n[FAIL] Connection reset by peer. Your server closed the fd after the first response.")
    except BrokenPipeError:
        print("\n[FAIL] Broken pipe. Server disconnected before the second request could be sent.")
    except Exception as e:
        print(f"\n[ERROR] An unexpected error occurred: {e}")

if __name__ == "__main__":
    test_keep_alive("127.0.0.1", 8080)