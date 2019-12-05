import sys
import grpc
from google.protobuf import empty_pb2
import engine_pb2
import engine_pb2_grpc


def run(command):
    channel = grpc.insecure_channel('localhost:50051')
    stub = engine_pb2_grpc.EngineStub(channel)
    if command == "GetVersion":
        version = stub.GetVersion(empty_pb2.Empty())
        print("GetVersion:", version.major, version.minor, version.patch)
    else:
        sys.exit("Error: {} is an unknown command".format(command))


if len(sys.argv) < 2:
    sys.exit("Error: usage: client.py <COMMAND>")
run(sys.argv[1])
