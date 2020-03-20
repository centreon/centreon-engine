import time
import grpc
import engine_pb2
import engine_pb2_grpc
from google.protobuf import empty_pb2, timestamp_pb2

def run():
    with grpc.insecure_channel("127.0.0.1:50051") as channel:
        stub = engine_pb2_grpc.EngineStub(channel)
        check = stub.NewThresholdsFile(engine_pb2.ThresholdsFile(
                                       filename="/etc/centreon-engine/ad.json"))
run()
