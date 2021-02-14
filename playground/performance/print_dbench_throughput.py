import sys
import os
import re

import copy
import math

configs = ['baseline.noopt', '+retpolines.noopt.old', '+retpolines.pibe-opt.lmbench-work.999990.old', '+retpolines.system.noopt', '+retpolines+retretpolines.noopt', '+retpolines+retretpolines.pibe-opt.lmbench-work.990000', '+retpolines+retretpolines.pibe-opt.lmbench-work.999000', '+retpolines+retretpolines.pibe-opt.lmbench-work.999999', 'alldefenses.noopt', 'alldefenses.pibe-opt.lmbench-work.990000', 'alldefenses.pibe-opt.lmbench-work.999000', 'alldefenses.pibe-opt.lmbench-work.999999', '+lvi.noopt', '+lvi.pibe-opt.lmbench-work.999999', '+retretpolines.noopt',  '+retretpolines.pibe-opt.lmbench-work.999999']
#configs= ['baseline.noopt', '+retpolines.noopt.old', '+retpolines.pibe-opt.lmbench-work.999990.old', '+retpolines.system.noopt',  '+retpolines+retretpolines.noopt', '+retpolines+retretpolines.pibe-opt.lmbench-work.990000']
#primary_folder='apache_remote_wired_600_100000'
#primary_folder='apache_remote_wired'
times=1
mid= int(times/2)
def parseDBenchFile(name):
    inputFile =  open(name,encoding="utf8", errors='ignore')
    for line in inputFile:
        if 'Throughput' in line:
            inputFile.close()
            #print(line.split(':')[1].split(" ")[4])
            return float(line.split(" ")[1])
     
    inputFile.close()
    return 0


def parseDBenchFolder(folder):
    med = 0.0
    for conf in configs:
      throughput = parseDBenchFile(folder+"/"+"dbench."+conf)
      if (conf == 'baseline.noopt'):
          med = throughput
      print(conf+": "+str(throughput)+ " "+ str(round(((throughput/med)-1)*100,2)) +"%" )

def main():
   print(sys.argv[1])
   parseDBenchFolder(sys.argv[1])
   print(mid)


main()

