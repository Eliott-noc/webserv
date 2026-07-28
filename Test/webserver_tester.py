import socket
import time
import re
import sys
import asyncio
import argparse

try:
    import requests
    HAS_REQUESTS = True
except ImportError:
    HAS_REQUESTS = False

try:
    import aiohttp
    HAS_AIOHTTP = True
except ImportError:
    HAS_AIOHTTP = False

TARGET_HOST = "127.0.0.1"
TARGET_PORT = 8080
BASE_URL = f"http://{TARGET_HOST}:{TARGET_PORT}"

# ==========================================
# HELPER FUNCTIONS
# ==========================================

def recv_full_response(sock, timeout=5.0):
    """Reads headers until \r\n\r\n, extracts Content-Length, and reads the full body."""
    sock.settimeout(timeout)
    response_data = b""
    
    while b"\r\n\r\n" not in response_data:
        try:
            chunk = sock.recv(4096)
            if not chunk:
                break
            response_data += chunk
        except socket.timeout:
            return None, b""
            
    if not response_data:
        return None, b""

    headers_raw, body_raw = response_data.split(b"\r\n\r\n", 1)
    headers = headers_raw.decode('utf-8', errors='ignore')
    
    content_length = 0
    match = re.search(r'(?i)Content-Length:\s*(\d+)', headers)
    if match:
        content_length = int(match.group(1))
        
    body = body_raw
    while len(body) < content_length:
        try:
            chunk = sock.recv(4096)
            if not chunk:
                break
            body += chunk
        except socket.timeout:
            break
            
    return headers, body

def send_raw_request(name, raw_request, expected_status_prefixes):
    """Sends a raw string over a socket and checks the status line."""
    print(f"  -> Running: {name}")
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(2.0)
        s.connect((TARGET_HOST, TARGET_PORT))
        s.sendall(raw_request.encode('utf-8'))
        
        response = s.recv(4096).decode('utf-8', errors='ignore')
        s.close()
        
        status_line = response.split('\r\n')[0] if response else "No Response"
        
        if status_line.startswith(expected_status_prefixes):
            print(f"     ✅ PASS (Got: {status_line})")
        else:
            print(f"     ❌ FAIL (Got: '{status_line}', Expected one of {expected_status_prefixes})")
    except Exception as e:
        print(f"     ❌ ERROR: {e}")

# ==========================================
# 1. CONNECTION & STATE TESTS
# ==========================================

def run_connection_tests():
    print("\n" + "="*40 + "\n1. CONNECTION & STATE TESTS\n" + "="*40)
    
    # Keep-Alive
    print("  -> Running: Keep-Alive Validation")
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((TARGET_HOST, TARGET_PORT))
        
        s.sendall(f"GET / HTTP/1.1\r\nHost: {TARGET_HOST}\r\nConnection: keep-alive\r\n\r\n".encode())
        headers1, body1 = recv_full_response(s)
        
        if headers1:
            s.sendall(f"GET / HTTP/1.1\r\nHost: {TARGET_HOST}\r\nConnection: close\r\n\r\n".encode())
            headers2, body2 = recv_full_response(s)
            if headers2 and "HTTP/" in headers2:
                print("     ✅ PASS: Server handled multiple requests on the same connection.")
            else:
                print("     ❌ FAIL: Server dropped connection before second request.")
        else:
            print("     ❌ FAIL: No response to first request.")
        s.close()
    except Exception as e:
        print(f"     ❌ ERROR: {e}")

    # Slowloris
    print("  -> Running: Slowloris (Partial Headers)")
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((TARGET_HOST, TARGET_PORT))
        s.sendall(b"GET / HTTP/1.1\r\nHost: localhost\r\n")
        time.sleep(3) 
        s.sendall(b"X-Test-Header: Done\r\n\r\n")
        
        response = s.recv(4096).decode('utf-8', errors='ignore')
        s.close()
        if response.startswith("HTTP/1.1"):
            print("     ✅ PASS: Server did not block and responded.")
        else:
            print("     ❌ FAIL: Invalid response.")
    except ConnectionResetError:
        print("     ✅ PASS (Timeout): Server dropped stagnant connection.")
    except Exception as e:
        print(f"     ❌ ERROR: {e}")

    # Premature Disconnect
    print("  -> Running: Premature Disconnect (Ghost Client)")
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((TARGET_HOST, TARGET_PORT))
        s.sendall(b"GET / HTT")
        s.close()
        print("     ✅ Sent partial request and closed FD. Check C++ server logs to ensure no crash/SIGPIPE!")
    except Exception as e:
        print(f"     ❌ ERROR: {e}")

# ==========================================
# 2. RAW HTTP PROTOCOL & SECURITY TESTS
# ==========================================

def run_raw_tests():
    print("\n" + "="*40 + "\n2. RAW HTTP PROTOCOL & SECURITY TESTS\n" + "="*40)
    
    req1 = "GET / HTTP/1.1\r\n\r\n"
    send_raw_request("Missing Host Header", req1, ("HTTP/1.1 400",))

    req2 = "GET /../../../../etc/passwd HTTP/1.1\r\nHost: localhost\r\n\r\n"
    send_raw_request("Path Traversal", req2, ("HTTP/1.1 403", "HTTP/1.1 404", "HTTP/1.1 400"))

    req3 = "INVALIDMETHOD / HTTP/1.1\r\nHost: localhost\r\n\r\n"
    send_raw_request("Unknown Method", req3, ("HTTP/1.1 501", "HTTP/1.1 405", "HTTP/1.1 400"))

    huge_path = "/" + ("A" * 8000)
    req4 = f"GET {huge_path} HTTP/1.1\r\nHost: localhost\r\n\r\n"
    send_raw_request("Huge URI (Buffer limits)", req4, ("HTTP/1.1 414", "HTTP/1.1 400"))

    # NEW TEST 1: Massive Content-Length Body Overflow
    req5 = "POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 999999999\r\n\r\nshort_body"
    send_raw_request("Massive Content-Length Protection", req5, ("HTTP/1.1 413", "HTTP/1.1 400"))

    # NEW TEST 2: Chunked Transfer Encoding Basic Formatting
    req6 = "POST / HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nhello\r\n6\r\n world\r\n0\r\n\r\n"
    send_raw_request("Chunked Transfer Encoding Parsing", req6, ("HTTP/1.1 200", "HTTP/1.1 201", "HTTP/1.1 204", "HTTP/1.1 400", "HTTP/1.1 405"))

# ==========================================
# 3. ROUTING & HIGH-LEVEL TESTS
# ==========================================

def run_routing_tests():
    print("\n" + "="*40 + "\n3. ROUTING & HIGH-LEVEL TESTS\n" + "="*40)
    if not HAS_REQUESTS:
        print("  ⚠️ Skipping: 'requests' library not installed.")
        return

    print("  -> Running: Homepage 200 OK")
    try:
        resp = requests.get(f"{BASE_URL}/")
        if resp.status_code == 200:
            print("     ✅ PASS")
        else:
            print(f"     ❌ FAIL (Got: {resp.status_code})")
    except Exception as e:
        print(f"     ❌ ERROR: {e}")

    print("  -> Running: 404 Not Found")
    try:
        resp = requests.get(f"{BASE_URL}/this-route-is-fake-1234")
        if resp.status_code == 404:
            print("     ✅ PASS")
        else:
            print(f"     ❌ FAIL (Got: {resp.status_code})")
    except Exception as e:
        print(f"     ❌ ERROR: {e}")

# ==========================================
# 4. STRESS TEST (ASYNC)
# ==========================================

async def _fetch(session, url):
    start = time.perf_counter()
    try:
        async with session.get(url) as response:
            await response.read()
            return response.status, time.perf_counter() - start
    except Exception:
        return 0, time.perf_counter() - start

async def _bound_fetch(sem, session, url):
    async with sem:
        return await _fetch(session, url)

async def _run_stress(total_reqs, concurrency):
    print(f"  -> Sending {total_reqs} requests (max {concurrency} concurrent)...")
    sem = asyncio.Semaphore(concurrency)
    async with aiohttp.ClientSession() as session:
        tasks = [_bound_fetch(sem, session, BASE_URL) for _ in range(total_reqs)]
        start_time = time.perf_counter()
        results = await asyncio.gather(*tasks)
        total_time = time.perf_counter() - start_time
        
    success_count = sum(1 for status, _ in results if status == 200)
    error_count = total_reqs - success_count
    avg_time = sum(t for _, t in results) / len(results)
    
    print(f"     Time:       {total_time:.2f}s ({total_reqs / total_time:.0f} req/s)")
    print(f"     Success:    {success_count}")
    print(f"     Errors:     {error_count}")
    print(f"     Avg Latency: {avg_time * 1000:.2f}ms")
    
    if error_count == 0:
        print("     ✅ PASS: Server survived the stress test flawlessly.")
    else:
        print("     ⚠️ WARN: Server dropped some connections under load.")

def run_stress_test(total_reqs=2000, concurrency=150):
    print("\n" + "="*40 + "\n4. STRESS TEST\n" + "="*40)
    if not HAS_AIOHTTP:
        print("  ⚠️ Skipping: 'aiohttp' library not installed.")
        return
        
    if sys.version_info[0] == 3 and sys.version_info[1] >= 8 and sys.platform.startswith('win'):
        asyncio.set_event_loop_policy(asyncio.WindowsSelectorEventLoopPolicy())
        
    asyncio.run(_run_stress(total_reqs, concurrency))

# ==========================================
# MAIN EXECUTION
# ==========================================

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Webserv Mega Tester")
    parser.add_argument("--conn", action="store_true", help="Run connection/state tests")
    parser.add_argument("--raw", action="store_true", help="Run raw protocol tests")
    parser.add_argument("--route", action="store_true", help="Run high-level routing tests")
    parser.add_argument("--stress", action="store_true", help="Run async stress tests")
    
    args = parser.parse_args()
    
    run_all = not any(vars(args).values())
    
    print(f"🚀 WEBSERV TESTER INITIATED (Target: {BASE_URL})")
    
    if run_all or args.conn:
        run_connection_tests()
    if run_all or args.raw:
        run_raw_tests()
    if run_all or args.route:
        run_routing_tests()
    if run_all or args.stress:
        run_stress_test()
        
    print("\n🎯 TESTING COMPLETE.\n")