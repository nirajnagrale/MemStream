import asyncio
import sys
from websockets import connect

async def main():
    uri = "ws://localhost:9002"
    try:
        async with connect(uri) as websocket:
            print(f"Connected to {uri}. Listening for messages...")
            async for message in websocket:
                print(message)
    except Exception as e:
        print(f"Connection error: {e}", file=sys.stderr)

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except RuntimeError:
        loop = asyncio.get_event_loop()
        loop.run_until_complete(main())
