#!/usr/bin/env python3
import socket

HOST = "localhost"
PORT = 9002

def main():
    # Create UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    
    try:
        sock.bind((HOST, PORT))
        print(f"[{PORT}] UDP client listening on {HOST}:{PORT}...")
        
        # Listen for messages
        while True:
            data, addr = sock.recvfrom(1024)  
            message = data.decode('utf-8')
            print(f"[{PORT}] Received from {addr}: {message!r}")
            
    except KeyboardInterrupt:
        print(f"\n[{PORT}] Shutting down...")
    except Exception as e:
        print(f"[{PORT}] Error: {e}")
    finally:
        sock.close()

if __name__ == "__main__":
    main()