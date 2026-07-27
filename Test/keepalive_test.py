import http.client
import time
import sys

def test_keep_alive(host, port, path="/"):
    print(f"Testing Keep-Alive on {host}:{port}{path}\n")
    
    # Create a single HTTP connection object
    conn = http.client.HTTPConnection(host, port, timeout=5)
    
    # Explicitly request Keep-Alive
    headers = {
        "Connection": "keep-alive"
    }
    
    try:
        # --- REQUEST 1 ---
        print("--- Request 1 ---")
        conn.request("GET", path, headers=headers)
        response1 = conn.getresponse()
        
        # We MUST read the response body fully to free the socket for the next request
        response1.read() 
        
        print(f"Status: {response1.status} {response1.reason}")
        print(f"Server 'Connection' Header: {response1.getheader('Connection', 'Not provided by server')}")
        
        # Grab the local port of our underlying TCP socket
        local_port1 = conn.sock.getsockname()[1]
        print(f"Local TCP Port used: {local_port1}\n")
        
        # Wait a moment to ensure the server doesn't immediately close idle connections
        print("Waiting 2 seconds...")
        time.sleep(2)
        print("Done.\n")
        
        # --- REQUEST 2 ---
        print("--- Request 2 ---")
        # If the server closed the connection, sending this next request 
        # will throw a RemoteDisconnected or ConnectionResetError exception.
        conn.request("GET", path, headers=headers)
        response2 = conn.getresponse()
        response2.read()
        
        print(f"Status: {response2.status} {response2.reason}")
        
        local_port2 = conn.sock.getsockname()[1]
        print(f"Local TCP Port used: {local_port2}\n")
        
        # --- VERDICT ---
        if local_port1 == local_port2:
            print("✅ SUCCESS: Both requests used the exact same TCP socket.")
            print("Your server successfully managed the Keep-Alive connection.")
        else:
            print("❌ FAILED: The connection was re-established on a new port.")
            
    except http.client.RemoteDisconnected:
        print("\n❌ FAILED: The server closed the connection after the first request (RemoteDisconnected).")
    except ConnectionResetError:
        print("\n❌ FAILED: The server actively reset the connection (RST packet received).")
    except Exception as e:
        print(f"\n⚠️ Error during test: {e}")
    finally:
        conn.close()

if __name__ == "__main__":
    # Change these variables to match your web server's environment
    TARGET_HOST = "127.0.0.1"
    TARGET_PORT = 8080
    TARGET_PATH = "/"
    
    test_keep_alive(TARGET_HOST, TARGET_PORT, TARGET_PATH)