# client_v2.py (second version)
# file to communicate with gRPC methods 

import json
import time
import grpc
import engine_pb2
import engine_pb2_grpc
from google.protobuf import descriptor, empty_pb2, timestamp_pb2
import sys
import inspect
from enum import Enum
from google.protobuf.json_format import Parse

# Class related with terminal colors
class colors:
  HEADER = '\033[95m'
  OKBLUE = '\033[94m'
  OKCYAN = '\033[96m'
  OKGREEN = '\033[92m'
  WARNING = '\033[93m'
  FAIL = '\033[91m'
  ENDC = '\033[0m'
  BOLD = '\033[1m'
  UNDERLINE = '\033[4m'

### Class ###

# Enum for grpc types
class gRPC_types(Enum):
  TYPE_DOUBLE      = 1
  TYPE_FLOAT       = 2 
  TYPE_INT64       = 3 
  TYPE_UINT64      = 4 
  TYPE_INT32       = 5
  TYPE_FIXEDINT64  = 6
  TYPE_FIXEDINT32  = 7
  TYPE_BOOL        = 8
  TYPE_STRING      = 9
  TYPE_GROUP       = 10
  TYPE_MESSAGE     = 11
  TYPE_BYTES       = 12
  TYPE_UTIN32      = 13
  TYPE_ENUM        = 14
  TYPE_SFIXED32    = 15
  TYPE_SFIXED64    = 16
  TYPE_SINT32      = 17
  TYPE_SINT64      = 18

# Client class that provides a communication with gRPC server
class gRPC_client:
  def __init__(self):
    self.stub = ""
    self.dic_methods = {}
    # Assiociate each method name with his descriptor
    for m in engine_pb2._ENGINE.methods: 
      self.dic_methods[m.name] = m

  def init_grpc(self, port):
    channel = grpc.insecure_channel("127.0.0.1:{}".format(port)) 
    self.stub = engine_pb2_grpc.EngineStub(channel)

  # Show list of gRPC methods 
  def show_list_grpc_methods(self):
    for m in engine_pb2._ENGINE.methods: 
      print("- ", m.name)

  # Get the method descriptor
  def get_grpc_method(self, method_name):
    try: 
      method = self.dic_methods[method_name]
    except KeyError:
      print("No method with this name found. Check the list of methods by using -l "
            "option")
      exit(1)

    return method

  # Give an info about grpc method
  def get_grpc_method_info(self, method_name):
    method = self.get_grpc_method(method_name) 
    # Get input type
    m_input = method.input_type
    # Same for output
    m_output = method.output_type

    lst_of_oneofs = []
    have_oneofs = False

    ## --- Description part --- ##
    print(colors.OKBLUE + colors.UNDERLINE + "DESCRIPTION:\n" + colors.ENDC)
    print("For method : {}, input parameter is : {}, output parameter is : {}."
          .format(method.name, m_input.name, m_output.name))
    print("Input Message {} contains the main fields:".format(m_input.name))
    
    for f in m_input.fields:
      print(" - {}".format(f.name))

    # Check if we have oneofs fields in our input
    if m_input.oneofs:
      have_oneofs = True
      oneofs_fields = m_input.oneofs_by_name[m_input.oneofs[0].name].fields
      for f in  oneofs_fields:
        lst_of_oneofs.append(f.name)

      print(colors.WARNING + "/!\ Note /!\\" + " fields: " + ', '.join(lst_of_oneofs) +            " are \'oneofs\', it means that you must choose one of the two but"
            " not the both !" + colors.ENDC, end='')

    ## --- Json Layout Part --- ## 
    print(colors.OKBLUE + colors.UNDERLINE + "\n\nJSON LAYOUT:\n" + colors.ENDC)

  # Launch a grpc method 
  def exe(self, method_name, message):
    try :
      str_to_eval = "self.stub." + method_name + "(message)"
      check = eval(str_to_eval)
    except grpc.RcpError as e:
      print(e.code())
    else:
      print(check)

### Basic Functions ###

# Show help/guide message
def help_message():
  print("Note that you need to inquire a Port in IPV6 for these uses :\n\n" 
        "-> python3 client.py <port> -h|--help : Show help message\n" 
        "-> python3 client.py <port> -d|--doc  : Show the documentation about"
        "this script and protobuf file (engine.proto)\n"
        "-> python3 client.py <port> -l|--list : Show the list of gRPC methods "
        "which are available\n"
        "-> python3 client.py <port> -i|--info <MethodName> : Show parameters about"
        "the method passed as argument to the script\n"
        "-> python3 client.py <port> -e|--exe <MethodName> "
        "<MessageContent.json>")

# Show documentation message
def documentation_message(): 
  print("You can read the gRPC_README.md to understand more about script working,"
        "you can read the documentation file \"index.html\" to see the documentation"
        "about protobuf file engine.proto")

 

# Function Arguments Errors
def arg_error(prog_name):
  print("Usage : python3 {} <port> -h|-d|-l|-i|-e".format(prog_name))
  help_message()
  exit(1)


### Main ###

if __name__ == "__main__":
  if len(sys.argv) < 3 or len(sys.argv) > 5:
    arg_error(sys.argv[0])

  # Init connection with the server and setup client class
  client = gRPC_client()
  client.init_grpc(sys.argv[1])
  
  # Get help message, doc and a list of grpc methods
  if len(sys.argv) == 3:
    if sys.argv[2] == "-h" or sys.argv[2] == "--help":
      help_message()
    elif sys.argv[2] == "-d" or sys.argv[2] == "--doc":
      documentation_message()
    elif sys.argv[2] == "-l" or sys.argv[2] == "--list":
      client.show_list_grpc_methods()
    else:
      arg_error(sys.argv[0])

  # Get info from a method
  if len(sys.argv) == 4:
    if sys.argv[2] == "-i" or sys.argv[2] == "--info":
      client.get_grpc_method_info(sys.argv[3]) 
    else:
      arg_error(sys.argv[0])


  # Execute a method 
  if len(sys.argv) == 5:
    if sys.argv[2] == "-e" or sys.argv[2] == "--exe":
      # Create a protobuf message by parsing json object
      with open(sys.argv[4]) as file:
        json_datas = json.load(file)

        m = client.get_grpc_method(sys.argv[3])
        mod = __import__('engine_pb2', fromlist=[m.input_type.name])
        c = getattr(mod, m.input_type.name)

        message = Parse(json.dumps(json_datas), c()) 
        client.exe(sys.argv[3], message)
    else:
      arg_error(sys.argv[0])
