from subprocess import Popen
import os
import signal
import sys


if len(sys.argv) == 1:
   print("Error supply configuration")
   sys.exit(-1)

command = ["./compile_install_kernel.sh", "hard-compile"]

configuration = sys.argv[1]
tokens = configuration.split(".")
print(tokens)

if (tokens[0] == "alldefenses"):
   command.append("+retpolines+lvi+retretpolines")
else:
   command.append(tokens[0])

if (tokens[1] == "noopt"):
   command.append("nooptimization")
elif (tokens[1] == "pibe-opt"):
   command.append("optimizations")
   if (tokens[2] == "apache-work"):
       command.append("apache2")
   elif (tokens[2] == "lmbench-work"):
       command.append("lmbench3")
   command.append(tokens[3])
    
print(command)
compiler = Popen(command, preexec_fn=os.setsid)

daemon = Popen(["python3", "python_daemon.py"])
error = daemon.wait()
if error == 5:
   print("Error restarting compilation...")
   os.killpg(os.getpgid(compiler.pid), signal.SIGTERM) 
   command[1] = "soft-compile"
   compiler = Popen(command)
else:
   print("No errors while compiling (closing daemon)")

error = compiler.wait()
