import time
import subprocess


def run():
  file = open("/var/log/centreon-engine/centengine.log")
  commandfile = "/var/lib/centreon-engine/rw/centengine.cmd"

  tps1 =  time.time()
  for i in range(10000):
    now = int(time.time())
    bashcmd = "/bin/printf \"[%lu] ADD_HOST_COMMENT;host;0;admin;Benchmark " + str(i) + " \n\"" + str(now) + " > " + str(commandfile) 
    output = subprocess.check_output(['bash', '-c', bashcmd])


  last_line = file.readlines()[-1]
  while last_line.find("Benchmark 99") < 0:
    file = open("/var/log/centreon-engine/centengine.log")
    last_line = file.readlines()[-1]

  tps2 =  time.time()
  print(tps2 - tps1)
  file.close()
run()
