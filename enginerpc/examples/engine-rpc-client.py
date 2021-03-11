# client_v2.py (second version)
# file to communicate with gRPC methods 

import inspect, sys, getopt, time, grpc, json
import engine_pb2
import engine_pb2_grpc
from google.protobuf import descriptor, empty_pb2, timestamp_pb2
from google.protobuf.json_format import Parse
from enum import Enum
from collections import namedtuple

### Class ###

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

  def init_grpc(self, ip, port):
    channel = grpc.insecure_channel("{}:{}".format(ip, port)) 
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

      print(colors.WARNING + "/!\ Note /!\\" + " fields: " + ', '.join(lst_of_oneofs) +  
            " are \'oneofs\', it means that you must choose one of the two but"
            " not the both !" + colors.ENDC, end='')

    ## --- Json Layout Part --- ## 
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


  # Launch a grpc method 
  def exe(self, method_name, message):
    try :
      str_to_eval = "self.stub." + method_name + "(message)"
      check = eval(str_to_eval)
    except grpc.RpcError as e:
      print(e.code())
    else:
      print(check)

### Basic Functions ###

# Convert a json object into a gRPC message
def json_to_message(client, method_name, json_datas):
  m = client.get_grpc_method(method_name)
  if m.input_type.name == "Empty":
    mod = __import__('google.protobuf.empty_pb2', fromlist=[m.input_type.name])
  else:
    mod = __import__('engine_pb2', fromlist=[m.input_type.name])

  c = getattr(mod, m.input_type.name)
  message = Parse(json.dumps(json_datas), c()) 

  return message

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

def check_arguments(client, args, flags):
  # check if we have one flags and not more
  if sum(flags._asdict().values()) > 1:
    print(colors.WARNING + "/!\ Warning /!\\ You have probably used at least two "
        "of these options (-l|--list, -d|--description, -h|--help, -e|--exe) in the "
        "same command line, you must only use one !\n\n" + colors.ENDC)
    exit(1)

  if flags.LIST_METHOD:
    client.show_list_grpc_methods()
  elif flags.HELP_METHOD:
    help_message()
  elif flags.DESCRIPTION_METHOD:
    client.get_grpc_method_info(args.method_name)
  elif flags.EXEC_METHOD:
    if not args.port:
      print(colors.WARNING + "/!\ Warning /!\\ Port is not defined" + colors.ENDC)
      exit(1)
    client.init_grpc(args.ip, args.port)

    # We probably should have an empty message.
    if not args.input_file and not args.json_args:
      m = client.get_grpc_method(args.method_name)

      if m.input_type.name == "Empty":
        mod = __import__('google.protobuf.empty_pb2', fromlist=[m.input_type.name])
        c = getattr(mod, m.input_type.name)
        json_datas = json.loads("{}")
        message = Parse(json.dumps(json_datas), c()) 
        client.exe(args.method_name, message)
      else:
        print(colors.WARNING + "/!\ Warning /!\ Your method have not Empty in his input field but you have not \n"
              "entered json input file and no json arguments.\n" + colors.ENDC)

    if args.json_args:
      json_datas = json.loads(args.json_args)
      msg = json_to_message(client, args.method_name, json_datas)
      client.exe(args.method_name, msg)

    if args.input_file:
      with open(args.input_file) as file:
        json_datas = json.load(file)
        msg = json_to_message(client, args.method_name, json_datas)
        client.exe(args.method_name, msg)

    

### Main ###
if __name__ == "__main__":
  # Defines flags 
  Arguments = namedtuple("Arguments", "ip, port, input_file, json_args, method_name")
  Flags     = namedtuple("Flags", "LIST_METHOD, HELP_METHOD, DESCRIPTION_METHOD, EXEC_METHOD")

  arguments_fields =  Arguments(ip="127.0.0.1", port="", input_file="", json_args="", method_name="")
  flags_fields     =  Flags(LIST_METHOD=False, HELP_METHOD=False, DESCRIPTION_METHOD=False, EXEC_METHOD=False)
  ip          = "127.0.0.1"
  port        = ""
  input_file  = ""
  json_args   = ""
  client      = gRPC_client()

  try:
    opts, args = getopt.getopt(sys.argv[1:], "hlp:f:a:d:e:",
                              ["help", "list", "port=", "file=", 
                              "args=", "description=", "exe="])
  except getopt.GetoptError as err:
    print(err)
    arg_error(sys.argv[0])

  # Parsing essential options. 
  # this allows to reverse the order of the arguments in the reading
  for o, a in opts:
    if o in ("-i", "--ip"):
      ip = a
      arguments_fields._replace(ip=a)
    elif o in ("-p", "--port"):
      port = a
      arguments_fields = arguments_fields._replace(port=a)
    elif o in ("-a", "--args"):
      json_args = a
      arguments_fields = arguments_fields._replace(json_args=a)
    elif o in ("-f", "--file"):
      input_file = a
      arguments_fields = arguments_fields._replace(input_file=a)
    elif o in ("-l", "--list"):
      flags_fields = flags_fields._replace(LIST_METHOD=True)
    elif o in ("-h", "--help"):
      flags_fields = flags_fields._replace(HELP_METHOD=True)
    elif o in ("-d", "--description"):
      arguments_fields = arguments_fields._replace(method_name=a)
      flags_fields = flags_fields._replace(DESCRIPTION_METHOD=True)
    elif o in ("-e", "--exe"):
      arguments_fields = arguments_fields._replace(method_name=a)
      flags_fields = flags_fields._replace(EXEC_METHOD=True)

  check_arguments(client, arguments_fields, flags_fields)

  """
  # Parsing others. They executes script functions. 
  for o, a in opts:
    if o in ("-l", "--list"):
      client.show_list_grpc_methods()
    elif o in ("-h", "--help"):
      help_message()
    elif o in ("-d", "--description"):
      method_name = a
      client.get_grpc_method_info(method_name)
    elif o in ("-e", "--exe"):
      method_name = a
      if not port:
        print("port is not defined\n")
        arg_error(sys.argv[0])
      client.init_grpc(ip, port)
      # We probably should have an empty message.
      if not input_file and not json_args:
        m = client.get_grpc_method(method_name)
          
        if m.input_type.name == "Empty":
          mod = __import__('google.protobuf.empty_pb2', fromlist=[m.input_type.name])
          c = getattr(mod, m.input_type.name)
          json_datas = json.loads("{}")
          message = Parse(json.dumps(json_datas), c()) 
          client.exe(method_name, message)
        else:
          print("Your method have not Empty in his input field but you have not \n"
                "entered json input file and no json arguments.\n")
      if json_args:
        json_datas = json.loads(json_args)
        msg = json_to_message(client, method_name, json_datas)
        client.exe(method_name, msg)

      if input_file:
        with open(input_file) as file:
          json_datas = json.load(file)
          msg = json_to_message(client, method_name, json_datas)
          client.exe(method_name, msg)
  """
