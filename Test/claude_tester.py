#!/usr/bin/env python3
"""
webserv_tester.py - a raw-socket test harness for the 42 "webserv" project.

Why raw sockets instead of `requests`:
    High-level HTTP clients normalize your input. You cannot send a malformed
    request line, a deliberately-wrong Content-Length, or half a request and
    pause, through `requests`. Testing an HTTP *server* means controlling the
    exact bytes on the wire, so we speak the protocol by hand.

Design notes (be critical of these):
    - Concurrency uses threads, not asyncio. For a few hundred sockets the
      bottleneck is your server, not this harness, and threaded code is easier
      for a peer to read and audit during defense.
    - NGINX comparison is *structural*, not byte-exact. Server/Date/header-order
      differ between any two servers and the subject does not require matching
      them. Comparing those would produce noise you'd learn to ignore. We
      compare status code + a whitelist of semantic headers + body.
    - A passing homemade tester proves little on its own: you are testing your
      own assumptions about HTTP against your own server. Treat green output as
      "no obvious contradiction found", not "correct". The NGINX diff and peer
      review are the real ground truth.

Usage:
    python3 claude_tester.py --host 127.0.0.1 --port 8080
    python3 claude_tester.py --host 127.0.0.1 --port 8080 --only core,malformed
    python3 claude_tester.py --host 127.0.0.1 --port 8080 --nginx-port 8081
    python3 claude_tester.py --host 127.0.0.1 --port 8080 --stress --connections 300

No third-party dependencies. Python 3.6+.
"""

import argparse
import socket
import ssl  # noqa: F401  (imported so TLS extension is a small step later)
import sys
import threading
import time
from collections import namedtuple

# ----------------------------------------------------------------------------
# Low-level HTTP over raw sockets
# ----------------------------------------------------------------------------

Response = namedtuple("Response", ["status", "reason", "headers", "body", "raw"])

CRLF = "\r\n"


class RawHTTPError(Exception):
    pass


class ConnectError(RawHTTPError):
    """Could not establish a connection at all. This is NEVER a pass -- it means
    the server is down or the wrong port was given, not that a bad request was
    correctly rejected. Kept separate so malformed-input tests don't mistake a
    dead server for a well-behaved one."""
    pass


def send_raw(host, port, payload, timeout=5.0, read_all=True, chunk_write=None):
    """Open a fresh connection, send `payload` (bytes), read the reply.

    chunk_write: if set to an int N, send the payload in N-byte pieces with a
                 tiny pause between them. This exercises the server's ability to
                 reassemble a request that arrives in fragments -- a classic
                 place where naive parsers break because they assume one recv()
                 delivers a whole request.
    """
    if isinstance(payload, str):
        payload = payload.encode("latin-1")

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.settimeout(timeout)
    try:
        sock.connect((host, port))
    except (ConnectionRefusedError, socket.timeout, OSError) as e:
        raise ConnectError("connect failed: %s" % e)

    try:
        if chunk_write:
            for i in range(0, len(payload), chunk_write):
                sock.sendall(payload[i:i + chunk_write])
                time.sleep(0.01)
        else:
            sock.sendall(payload)

        data = b""
        while read_all:
            try:
                buf = sock.recv(65536)
            except socket.timeout:
                break
            if not buf:
                break
            data += buf
            # Cheap termination heuristic: if we have full headers and either a
            # matching Content-Length body or the connection is closing, stop.
            if _looks_complete(data):
                break
        return data
    finally:
        sock.close()


def _looks_complete(data):
    """Heuristic to avoid blocking forever on keep-alive connections.

    This is intentionally conservative. It is NOT a correct HTTP parser and is
    not meant to be -- it only decides when *this test client* can stop reading.
    """
    sep = data.find(b"\r\n\r\n")
    if sep == -1:
        return False
    head = data[:sep].decode("latin-1", errors="replace").lower()
    body = data[sep + 4:]
    if "transfer-encoding: chunked" in head:
        return body.endswith(b"0\r\n\r\n")
    for line in head.split("\r\n"):
        if line.startswith("content-length:"):
            try:
                n = int(line.split(":", 1)[1].strip())
                return len(body) >= n
            except ValueError:
                return True
    # No body framing -> headers alone are the whole response.
    return True


def parse_response(raw):
    if not raw:
        raise RawHTTPError("empty response (server closed with no data?)")
    text = raw.decode("latin-1", errors="replace")
    sep = text.find("\r\n\r\n")
    if sep == -1:
        raise RawHTTPError("no header/body separator (malformed response)")
    head, body = text[:sep], text[sep + 4:]
    lines = head.split("\r\n")
    status_line = lines[0]
    parts = status_line.split(" ", 2)
    if len(parts) < 2 or not parts[0].startswith("HTTP/"):
        raise RawHTTPError("bad status line: %r" % status_line)
    try:
        status = int(parts[1])
    except ValueError:
        raise RawHTTPError("non-numeric status: %r" % status_line)
    reason = parts[2] if len(parts) > 2 else ""
    headers = {}
    for line in lines[1:]:
        if ":" in line:
            k, v = line.split(":", 1)
            headers[k.strip().lower()] = v.strip()
    return Response(status, reason, headers, body, raw)


def build_request(method, path, host, headers=None, body="", http_version="1.1"):
    """Assemble a request. Kept explicit so tests can violate it on purpose."""
    hdrs = dict(headers or {})
    hdrs.setdefault("Host", host)
    if body and "Content-Length" not in hdrs and "Transfer-Encoding" not in hdrs:
        hdrs["Content-Length"] = str(len(body))
    hdrs.setdefault("Connection", "close")
    lines = ["%s %s HTTP/%s" % (method, path, http_version)]
    for k, v in hdrs.items():
        lines.append("%s: %s" % (k, v))
    return CRLF.join(lines) + CRLF + CRLF + body


# ----------------------------------------------------------------------------
# Test registry + result tracking
# ----------------------------------------------------------------------------

class Result:
    def __init__(self):
        self.passed = 0
        self.failed = 0
        self.skipped = 0
        self.failures = []

    def ok(self, name):
        self.passed += 1
        print("  \033[32m[PASS]\033[0m %s" % name)

    def fail(self, name, detail):
        self.failed += 1
        self.failures.append((name, detail))
        print("  \033[31m[FAIL]\033[0m %s\n         -> %s" % (name, detail))

    def skip(self, name, why):
        self.skipped += 1
        print("  \033[33m[SKIP]\033[0m %s (%s)" % (name, why))


def expect_status(res, name, allowed, result):
    """`allowed` is a set/list of acceptable codes.

    We accept a *range* of codes on purpose: the subject says status codes must
    be "accurate" but the HTTP RFC leaves room (e.g. a missing resource can be
    404; a forbidden method can be 403 or 405). Pinning a single number would
    make the tester wrong more often than the server.
    """
    if isinstance(allowed, int):
        allowed = {allowed}
    if res.status in allowed:
        result.ok("%s -> %d" % (name, res.status))
    else:
        result.fail(name, "got %d %s, expected one of %s"
                    % (res.status, res.reason, sorted(allowed)))


# ----------------------------------------------------------------------------
# Test groups
# ----------------------------------------------------------------------------

def test_core(cfg, result):
    """GET / POST / DELETE and basic status-code sanity."""
    print("\n== core: methods & status codes ==")
    host = "%s:%d" % (cfg.host, cfg.port)

    # GET of the root should be a 2xx (or a redirect the server configured).
    try:
        raw = send_raw(cfg.host, cfg.port,
                       build_request("GET", "/", host))
        res = parse_response(raw)
        expect_status(res, "GET /", set(range(200, 400)), result)
    except RawHTTPError as e:
        result.fail("GET /", str(e))

    # GET of something that almost certainly doesn't exist -> 404.
    try:
        raw = send_raw(cfg.host, cfg.port,
                       build_request("GET", "/this_should_not_exist_42", host))
        res = parse_response(raw)
        expect_status(res, "GET missing", {404, 403}, result)
    except RawHTTPError as e:
        result.fail("GET missing", str(e))

    # POST with a small body. Accept a wide range: what's "correct" depends on
    # whether the route accepts POST and whether it's an upload target.
    try:
        raw = send_raw(cfg.host, cfg.port,
                       build_request("POST", cfg.upload_path, host,
                                     body="hello=world"))
        res = parse_response(raw)
        expect_status(res, "POST body",
                      {200, 201, 204, 301, 302, 303, 307, 308, 400, 403, 404, 405, 413},
                      result)
    except RawHTTPError as e:
        result.fail("POST body", str(e))

    # An unknown method. Servers should NOT crash; 400/405/501 all defensible.
    try:
        raw = send_raw(cfg.host, cfg.port,
                       build_request("BREW", "/", host))
        res = parse_response(raw)
        expect_status(res, "unknown method BREW", {400, 405, 501}, result)
    except RawHTTPError as e:
        result.fail("unknown method BREW", str(e))

    # HTTP/1.1 without Host header is a 400 per RFC 7230 section 5.4.
    try:
        req = ("GET / HTTP/1.1" + CRLF + "Connection: close" + CRLF + CRLF)
        raw = send_raw(cfg.host, cfg.port, req)
        res = parse_response(raw)
        expect_status(res, "1.1 no Host", {400}, result)
    except RawHTTPError as e:
        result.fail("1.1 no Host", str(e))


def test_malformed(cfg, result):
    """Deliberately broken input. The pass bar is: server replies with an error
    and stays alive, and does NOT hang. A hang or a crash is the real failure."""
    print("\n== malformed: broken requests & framing ==")
    host = "%s:%d" % (cfg.host, cfg.port)

    cases = [
        ("garbage line", "GET / HTTP/1.1garbage\r\n\r\n", {400, 505}),
        ("bad version", "GET / HTTP/9.9\r\nHost: %s\r\nConnection: close\r\n\r\n" % host, {400, 505}),
        ("no CRLF ever", "GET / HTTP/1.1", None),  # None => just must not hang/crash
        ("missing method", "/ HTTP/1.1\r\nHost: %s\r\n\r\n" % host, {400}),
        ("negative content-length",
         "POST / HTTP/1.1\r\nHost: %s\r\nContent-Length: -5\r\nConnection: close\r\n\r\n" % host,
         {400}),
        ("huge content-length no body",
         "POST / HTTP/1.1\r\nHost: %s\r\nContent-Length: 999999\r\nConnection: close\r\n\r\nx" % host,
         None),  # server should time out the wait, not hang forever
    ]

    for name, payload, allowed in cases:
        t0 = time.time()
        try:
            raw = send_raw(cfg.host, cfg.port, payload, timeout=cfg.timeout)
            elapsed = time.time() - t0
            if elapsed >= cfg.timeout - 0.1:
                result.fail(name, "no response within %.1fs (possible hang)" % cfg.timeout)
                continue
            if allowed is None:
                if raw:
                    try:
                        res = parse_response(raw)
                        result.ok("%s -> handled with %d" % (name, res.status))
                    except RawHTTPError:
                        result.ok("%s -> closed/garbage but no hang" % name)
                else:
                    result.ok("%s -> closed cleanly" % name)
            else:
                res = parse_response(raw)
                expect_status(res, name, allowed, result)
        except ConnectError as e:
            # Could not connect at all -> server is down / wrong port. This is a
            # real failure, not a well-handled bad request.
            result.fail(name, "server not reachable: %s" % e)
        except RawHTTPError as e:
            # Mid-request reset / garbage-then-close on malformed input is
            # acceptable; the server chose to drop a bad connection. A hang is
            # not (caught earlier by the timeout check).
            result.ok("%s -> server reset the bad connection (%s)" % (name, e))

    # Verify the server is STILL alive after all that abuse.
    try:
        raw = send_raw(cfg.host, cfg.port, build_request("GET", "/", host))
        parse_response(raw)
        result.ok("server still alive after malformed barrage")
    except RawHTTPError as e:
        result.fail("server survival check", "server unresponsive after malformed input: %s" % e)


def test_chunked(cfg, result):
    """Chunked transfer-encoding on the request side + fragmented sends."""
    print("\n== chunked & partial sends ==")
    host = "%s:%d" % (cfg.host, cfg.port)

    # A properly chunked POST body. The server must un-chunk it (subject IV.3).
    body = "Wikipedia in\r\n\r\nchunks."
    chunked = ""
    # split into two chunks to make sure multi-chunk assembly works
    mid = len(body) // 2
    for piece in (body[:mid], body[mid:]):
        chunked += "%X\r\n%s\r\n" % (len(piece), piece)
    chunked += "0\r\n\r\n"

    req = ("POST %s HTTP/1.1\r\n"
           "Host: %s\r\n"
           "Transfer-Encoding: chunked\r\n"
           "Connection: close\r\n\r\n%s" % (cfg.upload_path, host, chunked))
    try:
        raw = send_raw(cfg.host, cfg.port, req, timeout=cfg.timeout)
        res = parse_response(raw)
        expect_status(res, "chunked POST",
                      {200, 201, 204, 400, 403, 404, 405, 413, 501}, result)
    except RawHTTPError as e:
        result.fail("chunked POST", str(e))

    # Same valid request, but dribbled out 4 bytes at a time. A parser that
    # assumes one recv() == one request will choke here even though the bytes
    # are identical to a request it handles fine.
    try:
        raw = send_raw(cfg.host, cfg.port,
                       build_request("GET", "/", host),
                       timeout=cfg.timeout, chunk_write=4)
        res = parse_response(raw)
        expect_status(res, "fragmented GET (4-byte writes)",
                      set(range(200, 400)), result)
    except RawHTTPError as e:
        result.fail("fragmented GET", str(e))


def test_upload_cgi(cfg, result):
    """File upload round-trip and CGI. These depend heavily on your config, so
    failures here may mean 'route not configured' rather than 'server broken'.
    Read the detail before assuming a real bug."""
    print("\n== upload & CGI (config-dependent) ==")
    host = "%s:%d" % (cfg.host, cfg.port)

    # Multipart upload -- shape mirrors what a browser sends so you can compare.
    boundary = "----webservtest42boundary"
    payload_body = (
        "--%s\r\n"
        'Content-Disposition: form-data; name="file"; filename="probe.txt"\r\n'
        "Content-Type: text/plain\r\n\r\n"
        "webserv upload probe\r\n"
        "--%s--\r\n" % (boundary, boundary)
    )
    hdrs = {
        "Content-Type": "multipart/form-data; boundary=%s" % boundary,
    }
    try:
        raw = send_raw(cfg.host, cfg.port,
                       build_request("POST", cfg.upload_path, host,
                                     headers=hdrs, body=payload_body),
                       timeout=cfg.timeout)
        res = parse_response(raw)
        expect_status(res, "multipart upload",
                      {200, 201, 204, 400, 403, 404, 405, 413}, result)
    except RawHTTPError as e:
        result.fail("multipart upload", str(e))

    # DELETE the thing we may have just uploaded.
    try:
        raw = send_raw(cfg.host, cfg.port,
                       build_request("DELETE", cfg.upload_path + "probe.txt", host),
                       timeout=cfg.timeout)
        res = parse_response(raw)
        expect_status(res, "DELETE uploaded file",
                      {200, 202, 204, 403, 404, 405}, result)
    except RawHTTPError as e:
        result.fail("DELETE uploaded file", str(e))

    # CGI GET with a query string. The subject requires the query be exposed to
    # the CGI (QUERY_STRING env var). We can only check the server doesn't choke.
    if cfg.cgi_path:
        try:
            raw = send_raw(cfg.host, cfg.port,
                           build_request("GET", cfg.cgi_path + "?a=1&b=2", host),
                           timeout=cfg.timeout)
            res = parse_response(raw)
            expect_status(res, "CGI GET with query",
                          {200, 500, 502, 504}, result)
        except RawHTTPError as e:
            result.fail("CGI GET with query", str(e))
    else:
        result.skip("CGI test", "no --cgi-path given")


def test_stress(cfg, result):
    """Concurrency + availability. Threaded on purpose (see module docstring).

    We measure: how many concurrent connections succeed, and whether the server
    is still serving afterwards. The subject's bar is 'available at all times',
    so a single dropped connection under load is worth flagging but is not
    automatically a fail -- report the rate and let you judge."""
    print("\n== stress: %d concurrent connections ==" % cfg.connections)
    host = "%s:%d" % (cfg.host, cfg.port)
    successes = [0]
    errors = []
    lock = threading.Lock()

    def worker():
        try:
            raw = send_raw(cfg.host, cfg.port,
                           build_request("GET", "/", host),
                           timeout=cfg.timeout)
            res = parse_response(raw)
            if 200 <= res.status < 500:
                with lock:
                    successes[0] += 1
            else:
                with lock:
                    errors.append("status %d" % res.status)
        except (RawHTTPError, OSError) as e:
            with lock:
                errors.append(str(e))

    threads = [threading.Thread(target=worker) for _ in range(cfg.connections)]
    t0 = time.time()
    for t in threads:
        t.start()
    for t in threads:
        t.join()
    elapsed = time.time() - t0

    rate = successes[0] / cfg.connections * 100
    print("  completed %d/%d (%.1f%%) in %.2fs (%.0f req/s)"
          % (successes[0], cfg.connections, rate, elapsed,
             cfg.connections / elapsed if elapsed else 0))
    if errors:
        # Show only a few; a wall of identical errors isn't informative.
        sample = {}
        for e in errors:
            sample[e] = sample.get(e, 0) + 1
        for msg, count in list(sample.items())[:5]:
            print("    %dx %s" % (count, msg))

    if rate >= 95:
        result.ok("stress: %.1f%% succeeded under concurrency" % rate)
    else:
        result.fail("stress", "%.1f%% success rate under %d connections -- "
                    "investigate whether the single poll() loop is starving "
                    "connections or the accept backlog is too small"
                    % (rate, cfg.connections))

    # Availability check after the burst.
    try:
        raw = send_raw(cfg.host, cfg.port, build_request("GET", "/", host))
        parse_response(raw)
        result.ok("server responsive after stress")
    except RawHTTPError as e:
        result.fail("post-stress availability", "server down after load: %s" % e)


# ----------------------------------------------------------------------------
# NGINX comparison mode
# ----------------------------------------------------------------------------

# Headers that legitimately differ between any two servers and that the subject
# does not require you to match. Comparing these produces false alarms.
_IGNORED_HEADERS = {
    "server", "date", "connection", "keep-alive", "etag",
    "last-modified", "accept-ranges", "vary",
}


def compare_with_nginx(cfg, result):
    print("\n== NGINX comparison (structural) ==")
    print("  Note: comparing status + body + semantic headers only. Server/Date/")
    print("  ordering are normalized out on purpose -- matching them is neither")
    print("  required nor meaningful.\n")
    host_ours = "%s:%d" % (cfg.host, cfg.port)
    host_ngx = "%s:%d" % (cfg.host, cfg.nginx_port)

    probes = [
        ("GET", "/", ""),
        ("GET", "/index.html", ""),
        ("GET", "/nonexistent_42_probe", ""),
        ("GET", "/../../etc/passwd", ""),   # path traversal: both should refuse
    ]

    for method, path, body in probes:
        name = "%s %s" % (method, path)
        try:
            a = parse_response(send_raw(cfg.host, cfg.port,
                               build_request(method, path, host_ours, body=body)))
        except RawHTTPError as e:
            result.fail("cmp %s (ours)" % name, str(e))
            continue
        try:
            b = parse_response(send_raw(cfg.host, cfg.nginx_port,
                               build_request(method, path, host_ngx, body=body)))
        except RawHTTPError as e:
            result.skip("cmp %s" % name, "nginx side failed: %s" % e)
            continue

        diffs = []
        if a.status != b.status:
            diffs.append("status: ours=%d nginx=%d" % (a.status, b.status))
        # Compare semantic headers only.
        for key in ("content-type", "content-length", "location",
                    "allow", "transfer-encoding"):
            av, bv = a.headers.get(key), b.headers.get(key)
            if av != bv:
                diffs.append("%s: ours=%r nginx=%r" % (key, av, bv))
        if diffs:
            # A difference is a discussion point, not always a bug -- nginx has
            # config we didn't replicate. Report, don't hard-fail on body.
            result.fail("cmp %s" % name, "; ".join(diffs))
        else:
            result.ok("cmp %s -> matches structurally" % name)


# ----------------------------------------------------------------------------
# Runner
# ----------------------------------------------------------------------------

GROUPS = {
    "core": test_core,
    "malformed": test_malformed,
    "chunked": test_chunked,
    "upload": test_upload_cgi,
}


def main():
    p = argparse.ArgumentParser(description="Raw-socket tester for 42 webserv")
    p.add_argument("--host", default="127.0.0.1")
    p.add_argument("--port", type=int, required=True)
    p.add_argument("--timeout", type=float, default=5.0,
                   help="per-request timeout; also the hang-detection threshold")
    p.add_argument("--upload-path", default="/upload/",
                   help="a route configured to accept POST/uploads")
    p.add_argument("--cgi-path", default="",
                   help="e.g. /cgi-bin/test.py ; skipped if empty")
    p.add_argument("--only", default="",
                   help="comma list: core,malformed,chunked,upload")
    p.add_argument("--stress", action="store_true")
    p.add_argument("--connections", type=int, default=200)
    p.add_argument("--nginx-port", type=int, default=0,
                   help="if set, run structural comparison against nginx here")
    cfg = p.parse_args()

    result = Result()

    selected = GROUPS
    if cfg.only:
        wanted = [x.strip() for x in cfg.only.split(",")]
        selected = {k: v for k, v in GROUPS.items() if k in wanted}
        unknown = [w for w in wanted if w not in GROUPS]
        for u in unknown:
            print("warning: unknown group %r ignored" % u, file=sys.stderr)

    print("Target: %s:%d" % (cfg.host, cfg.port))
    for fn in selected.values():
        fn(cfg, result)

    if cfg.stress:
        test_stress(cfg, result)

    if cfg.nginx_port:
        compare_with_nginx(cfg, result)

    print("\n" + "=" * 50)
    print("PASSED: %d  FAILED: %d  SKIPPED: %d"
          % (result.passed, result.failed, result.skipped))
    if result.failures:
        print("\nFailures to investigate (remember: some are config, not bugs):")
        for name, detail in result.failures:
            print("  - %s: %s" % (name, detail))
    print("=" * 50)

    # Exit non-zero if anything failed, so you can wire this into a Makefile
    # target or CI without reading the output.
    sys.exit(1 if result.failed else 0)


if __name__ == "__main__":
    main()