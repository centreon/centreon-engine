import time
import grpc
import broker_pb2
import broker_pb2_grpc
from google.protobuf import empty_pb2, timestamp_pb2

def run():
    with grpc.insecure_channel("127.0.0.1:50051") as channel:
        stub = broker_pb2_grpc.BrokerStub(channel)
        for i in range(10000):
          check = stub.ProcessServiceCheckResult(broker_pb2.ServiceCheck(
                check_time=timestamp_pb2.Timestamp(seconds=int(time.time())),
                host_name="Central-titus",
                svc_desc="Cpu",
                output="Cpu with enginerpc",
                code=1))
run()
