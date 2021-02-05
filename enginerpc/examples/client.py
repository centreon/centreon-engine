# client.py
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

  # Get list of gRPC methods 
  def get_grpc_methods(self):
    for func in dir(self.stub):
      if callable(getattr(self.stub, func)) and not func.startswith("__"):
        print("- ", func)

  # Get the method descriptor
  def get_grpc_method(self, method_name):
    try: 
      method = self.dic_methods[method_name]
    except KeyError:
      print("No method with this name found. Check the list of methods by using -l"
             "option")
      exit(1)

    return method

  def get_grpc_method_info(self, method_name):
    method = self.get_grpc_method(method_name) 
    # Get input type
    m_input = method.input_type
    # Same for output
    m_output = method.output_type

    lst_of_oneofs = []
    have_oneofs = False

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
      print(colors.WARNING + "/!\ Note /!\\" + " fields: " + ', '.join(lst_of_oneofs) + 
            " are \'oneofs\', it means that you must choose one of the two but"
            " not the both !" + colors.ENDC, end='')

    print(colors.OKBLUE + colors.UNDERLINE + "\n\nJSON LAYOUT:\n" + colors.ENDC)
    result_str = "" 

    # Show to user how the input message looks like in json
    if have_oneofs:
      for o in lst_of_oneofs:
        print("Example :\n")
        print("{")
        result_str = ""
        for f in m_input.fields:
          if f.name == o:
            continue
          for type in gRPC_types:
            if f.type == type.value:
              # If we dont get a basic type (like int, string, etc..), 
              # it means that we have a type of TYPE_MESSAGE, so we have to describe 
              # the Message to get more informations about it.
              if f.type == gRPC_types.TYPE_MESSAGE.value:
                self.get_grpc_message_info(m_input, f, 3)
              # same for TYPE_ENUM
              elif f.type == gRPC_types.TYPE_ENUM.value:
                self.get_grpc_enum_info(m_input, f, 3)
              else:
                result_str += " \"{}\": {},\n".format(f.name, type.name)
        # Remove last '\n' and ',' characters
        if result_str:
          result_str = result_str[:-2]
          print(result_str)
        print("}\n")
        
    else:
      print("{")
      for f in m_input.fields:
        for type in gRPC_types:
          if f.type == type.value:
            # If we dont get a basic type (like int, string, etc..), 
            # it means that we have a type of TYPE_MESSAGE, so we have to describe 
            # the Message to get more informations about it.
            if f.type == gRPC_types.TYPE_MESSAGE.value:
              self.get_grpc_message_info(m_input, f, 3)
            # same for TYPE_ENUM
            elif f.type == gRPC_types.TYPE_ENUM.value:
              self.get_grpc_enum_info(m_input, f, 3)
            else:
              result_str += " \"{}\": {},\n".format(f.name, type.name)

      # Remove last '\n' and ',' characters
      if result_str:
        result_str = result_str[:-2]
        print(result_str)
      print("}")


  # Describe a TYPE_MESSAGE
  def get_grpc_message_info(self, parent_message_descriptor, field, string_space):
    # Get current message 
    current_msg_dsc = parent_message_descriptor.fields_by_name[field.name].message_type
    # str_format variable is used to indent text
    str_format = string_space * ' '
    print(str_format + "\"{}\":".format(field.name))

    str_format += ' '
    print(str_format + "{")
    result_str = "" 
    
    for f in current_msg_dsc.fields:
      for type in gRPC_types:
        if f.type == type.value:
          if f.type == gRPC_types.TYPE_MESSAGE.value:
            self.get_grpc_message_info(current_msg_dsc, f, n+1)
          elif f.type == gRPC_types.TYPE_ENUM.value:
            self.get_grpc_enum_info(m_input, f, 3)
          else:
            result_str += str_format + " \"{}\": {},\n".format(f.name, type.name)
     
    # Remove last '\n' and ',' characters
    result_str = result_str[:-2]
    print(result_str)
    str_format = ' '
    str_format += string_space * ' '
    print(str_format + "}")

  # Describe a TYPE_ENUM
  def get_grpc_enum_info(self, parent_message_descriptor, field, string_space):
    current_msg_dsc = parent_message_descriptor.fields_by_name[field.name].enum_type

    str_format = string_space * ' '
    print(str_format + "\"{}\":".format(field.name))

    str_format += ' '
    print(str_format + "{")

    for v in current_msg_dsc.values:
      print(str_format + " {}".format(v.name))
      
    str_format = ' '
    str_format += string_space * ' '
    print(str_format + "}")

  def show_help_msg(self):
    print("Note that you need to inquire a Port in IPV6 for these uses :\n\n" 
        "-> python3 client.py <port> -h|--help : Show help message\n" 
        "-> python3 client.py <port> -d|--doc  : Show the documentation about"
        "this script and protobuf file (engine.proto)\n"
        "-> python3 client.py <port> -l|--list : Show the list of gRPC methods "
        "which are available\n"
        "-> python3 client.py <port> -i|--info <MethodName> : Show parameters about"
        "the method passed as argument to the script\n"
        "-> python3 client.py <port> -e|--exe <MethodName> <MessageName> "
        "<MessageContent.json>")
  
  def show_doc_msg(self): 
    print("You can read the gRPC_README.md to understand more about script working,"
        "you can read the documentation file \"index.html\" to see the documentation"
        "about protobuf file engine.proto")

  # Launch a grpc function
  def exe(self, method_name, message):
    try :
      #str_to_eval = "self.stub." + method_name + """(engine_pb2.Check(check_time=timestamp_pb2.Timestamp(
      #              seconds=int(time.time())), host_name=\"Central-titus\", svc_desc=\"Cpu\", 
      #              output=\"cpu\", code=1))"""
      
      str_to_eval = "self.stub." + method_name + "(message)"
      #check = self.stub.GetHost(message)
      
      #Marche avec eval
      check = eval(str_to_eval)
   
    except grpc.RpcError as e:
      print(e.code())
    else:
      print(check)

if __name__ == "__main__":
  if len(sys.argv) < 3 or len(sys.argv) > 5:
    print("Usage : python3 {} <port> -h|-d|-l|-i|-e".format(sys.argv[0]))
    exit(1)
   
  # Init connection to the server and setup client class
  client = gRPC_client()
  client.init_grpc(sys.argv[1])
   
  # Get help message, doc and a list of grpc methods
  if len(sys.argv) == 3:
    if sys.argv[2]   == "-h" or sys.argv[2] == "--help":
      client.show_help_msg()
    elif sys.argv[2] == "-d" or sys.argv[2] == "--doc":
      client.show_doc_msg()
    elif sys.argv[2] == "-l" or sys.argv[2] == "--list":
      client.get_grpc_methods()
    else:
      print("Usage : python3 {} <port> -h|-d|-l|-i|-e".format(sys.argv[0]))
      client.show_help_msg()
      exit(1)

  # Get info from a method
  if len(sys.argv) == 4:
    if sys.argv[2] == "-i" or sys.argv[2] == "--info":
      client.get_grpc_method_info(sys.argv[3]) 
    else:
      print("Usage : python3 {} <port> -h|-d|-l|-i|-e".format(sys.argv[0]))
      client.show_help_msg()
      exit(1)

  # Execute a method 
  if len(sys.argv) == 5:
    # with file
    if sys.argv[2] == "-e" or sys.argv[2] == "--exe":
      # Create a protobuf message by parsing json object
      with open(sys.argv[4]) as file:
        json_datas = json.load(file)

        m = client.get_grpc_method(sys.argv[3])
        if m.input_type.name == "Empty":
          mod = __import__('google.protobuf.empty_pb2', fromlist=[m.input_type.name])
        else:
          mod = __import__('engine_pb2', fromlist=[m.input_type.name])

        c = getattr(mod, m.input_type.name)
        message = Parse(json.dumps(json_datas), c()) 
        client.exe(sys.argv[3], message)
    # directly in command line 
    elif sys.argv[2] == "-a":
        json_datas = json.loads(sys.argv[4])

        m = client.get_grpc_method(sys.argv[3])
        if m.input_type.name == "Empty":
          mod = __import__('google.protobuf.empty_pb2', fromlist=[m.input_type.name])
        else:
          mod = __import__('engine_pb2', fromlist=[m.input_type.name])

        c = getattr(mod, m.input_type.name)
        message = Parse(json.dumps(json_datas), c()) 
        client.exe(sys.argv[3], message)

    else:
      print("Usage : python3 {} <port> -h|-d|-l|-i|-e".format(sys.argv[0]))
      client.show_help_msg()
      exit(1)
