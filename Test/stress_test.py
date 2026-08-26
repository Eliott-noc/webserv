import asyncio
import aiohttp
import time
import sys

TARGET_URL = "http://127.0.0.1:8080/"
TOTAL_REQUESTS = 5000
CONCURRENCY_LIMIT = 200  # Number of simultaneous connections

async def fetch(session, url):
    start = time.perf_counter()
    try:
        async with session.get(url) as response:
            await response.read()
            return response.status, time.perf_counter() - start
    except Exception:
        # Catch connection drops, resets, or timeouts
        return 0, time.perf_counter() - start

async def bound_fetch(sem, session, url):
    # The semaphore restricts how many connections are active at the exact same time
    async with sem:
        return await fetch(session, url)

async def main():
    print(f"🚀 Starting stress test: {TOTAL_REQUESTS} requests, max {CONCURRENCY_LIMIT} concurrent...")
    sem = asyncio.Semaphore(CONCURRENCY_LIMIT)
    
    async with aiohttp.ClientSession() as session:
        tasks = [bound_fetch(sem, session, TARGET_URL) for _ in range(TOTAL_REQUESTS)]
        
        start_time = time.perf_counter()
        results = await asyncio.gather(*tasks)
        total_time = time.perf_counter() - start_time
        
    success_count = sum(1 for status, _ in results if status == 200)
    error_count = TOTAL_REQUESTS - success_count
    
    times = [t for _, t in results]
    avg_time = sum(times) / len(times)
    rps = TOTAL_REQUESTS / total_time
    
    print("\n--- Results ---")
    print(f"Total Time:    {total_time:.2f}s")
    print(f"Requests/sec:  {rps:.2f}")
    print(f"Success (200): {success_count}")
    print(f"Errors/Fails:  {error_count}")
    print(f"Avg Latency:   {avg_time * 1000:.2f}ms")

if __name__ == "__main__":
    # Cross-platform handling to prevent Event Loop crashes in some Windows environments
    if sys.version_info[0] == 3 and sys.version_info[1] >= 8 and sys.platform.startswith('win'):
        asyncio.set_event_loop_policy(asyncio.WindowsSelectorEventLoopPolicy())
    asyncio.run(main())