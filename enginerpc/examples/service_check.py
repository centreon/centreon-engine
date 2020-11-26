#!/usr/bin/python3
import time
import grpc
import engine_pb2
import engine_pb2_grpc
import sys
from google.protobuf import empty_pb2, timestamp_pb2

def run():
    with grpc.insecure_channel("127.0.0.1:{}".format(sys.argv[1])) as channel:
        stub = engine_pb2_grpc.EngineStub(channel)
        k = 0.0
        for j in range(100000):
          now = time.time()
          seconds = int(now)
          nanos = int((now - seconds) * 10**9)
          timestamp = timestamp_pb2.Timestamp(seconds=seconds, nanos=nanos)
          for i in range(1, 200):
              print("Step{}".format(i))
              check = stub.ProcessServiceCheckResult(engine_pb2.Check(
                    check_time=timestamp,
                    host_name = 'ceïntràl',
                    svc_desc = 'broker_' + str(i),
                    output = 'grpc check| grpc{}={}'.format(i, k),
                    code = 0))
          k += 0.1
          if k > 10:
            k = 0.0
run()
