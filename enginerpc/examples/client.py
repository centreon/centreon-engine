import grpc
import engine_pb2
import engine_pb2_grpc
from google.protobuf import empty_pb2

def run():
    with grpc.insecure_channel("10.0.2.15:50051") as channel:
        stub = engine_pb2_grpc.EngineStub(channel)
        version = stub.GetVersion(empty_pb2.Empty())
        print(version.major)

run()
