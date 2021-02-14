import time
import subprocess
import sys
from subprocess import Popen, PIPE
backup = ""
limit= 0
while True:
  time.sleep(120)
  output = Popen(["ls", "-l", "kernel/backport_kernel_dump"],stdout=PIPE)
  response = output.communicate()
  return_str = response[0].decode('ascii')
  if  (return_str == ""):
      continue
  catter = subprocess.Popen(["cat", "kernel/backport_kernel_dump"], stdout=subprocess.PIPE)
  grepper = subprocess.Popen(["grep", "scripts/link-vmlinux.sh"], stdin=catter.stdout, stdout=subprocess.PIPE)
  reached_vmlinux = grepper.communicate()[0].decode('ascii')
  if (reached_vmlinux != ""):
     sys.exit(0)
  timestr = return_str.split(" ")[7]
  if (timestr == backup):
     limit = limit+1
  else:
     limit = 0

  if (limit == 8):
     print("Nothing changed in last 16 minutes")
     sys.exit(5)
  backup = timestr
  
