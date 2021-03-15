import grpc, time
import engine_pb2
import engine_pb2_grpc
from google.protobuf import descriptor, empty_pb2, timestamp_pb2

def run():

  tps1 =  time.time()
  with grpc.insecure_channel("127.0.0.1:50970") as channel:
    stub = engine_pb2_grpc.EngineStub(channel)
    for i in range(10000):
      res = stub.AddHostComment(engine_pb2.EngineComment(host_name="host", 
                                      svc_desc="", user="admin", 
                                      comment_data="Benchmark" + str(i), persistent=0,
                                      entry_time=int(time.time())))

  tps2 =  time.time()
  print(tps2 - tps1)

run()


