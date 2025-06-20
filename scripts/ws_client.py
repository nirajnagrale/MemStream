#!/usr/bin/env python3
import asyncio
import websockets, sys

HOST = "localhost"
PORT = 9001

async def main():
    uri = f"ws://{HOST}:{PORT}"
    async with websockets.connect(uri) as ws:
        print(f"[{PORT}] Connected — listening for messages…")
        async for message in ws:
            print(f"[{PORT}] Received: {message!r}")

if __name__ == "__main__":
    asyncio.run(main())