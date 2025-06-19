import sys, zmq, time
if len(sys.argv)!=2 or sys.argv[1] not in ("TCP","UDP","BOTH"):
    print("Usage: controller.py [TCP|UDP|BOTH]"); sys.exit(1)
ctx=zmq.Context(); pub=ctx.socket(zmq.PUB); pub.bind("ipc:///tmp/control.ipc")
time.sleep(0.1)
pub.send_string(sys.argv[1]); print("Sent",sys.argv[1])
