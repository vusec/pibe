import sys
import os
import re

import copy
import math

#base='victor-X550JX.'

base='LMBench.'


import numpy as np

def geo_mean(iterable):
    a = np.array(iterable)
    return a.prod()**(1.0/len(a))



baselines = [ 'test.baseline.noopt']
benchmarks =  [ 'test.alldefenses.noopt', 'test.alldefenses.pibe-opt.lmbench-work.999999',  'test.alldefenses.pibe-opt.apache-work.999999', 'test.+retpolines.noopt' , 'test.+retpolines.pibe-opt.lmbench-work.999990' ,   'test.+retretpolines.noopt', 'test.+retretpolines.pibe-opt.lmbench-work.999999', 'test.+lvi.noopt', 'test.+lvi.pibe-opt.lmbench-work.999999' ]

allbaselines =  [ 'baseline.noopt' ]
allbenchmarks = [ 'alldefenses.noopt', 'alldefenses.pibe-opt.lmbench-work.999999',  'alldefenses.pibe-opt.apache-work.999999', '+retpolines.noopt' , '+retpolines.pibe-opt.lmbench-work.999990' ,   '+retretpolines.noopt', '+retretpolines.pibe-opt.lmbench-work.999999', '+lvi.noopt', '+lvi.pibe-opt.lmbench-work.999999' ]

if os.getenv("RUNS") is not None:
  numProfiles = int(os.environ["RUNS"]);
else:
  numProfiles = 5

numBaselines = 3

middle = int(numProfiles/2)

degree = 1

resultsSablon = {  "read" : [] , "write" : [] ,"open" : [] , "stat" : [] , "fstat" : [], "select_file" : [] , "select_tcp" : [] , "tcp" : [] , "udp" : [] , "tcp_conn" : [] , "af_unix" : [],  "pipe" : [] ,  "null" : [] , "page_fault" : [] , "mmap" : [], "fork/exit" : []  , "fork/exec" : [], "fork/shell" : [] , "sig_install" : [] , "sig_dispatch" : [] , "ctx_4k_pipe" : [] , "ctx_64k_pipe" : []}


showResultsOrder =  [ "null", "read" , "write", "open", "stat", "fstat", "af_unix", "fork/exit", "fork/exec", "fork/shell", "pipe", "select_file", "select_tcp", "tcp_conn", "udp", "tcp" , "mmap", "page_fault", "sig_install", "sig_dispatch"]

benchSelectionSablon =  [ "null", "read" , "write", "open", "stat", "fstat", "af_unix", "fork/exit", "fork/exec", "fork/shell", "pipe", "select_file", "select_tcp", "tcp_conn", "udp", "tcp" , "mmap", "page_fault", "sig_install", "sig_dispatch"]

retpolinesSelectionSablon = [ "null", "read", "write", "open", "stat", "fstat", "select_tcp", "udp" , "tcp", "tcp_conn" , "af_unix" , "pipe"]

tablesablon = []

    

def sortResultTable(table):
    for key in table:
        table[key].sort()

def computeMeanTable(table):
    sortResultTable(table)
    meanTable = {}
    for key in table:
        #print len(table[key])/2
        meanTable[key] = table[key][int(len(table[key])/2)]
    return meanTable

def computeMeanInstances(results):
    resultsV = []
    for instance in results:
        resultsV.append(computeMeanTable(instance))
    return resultsV

def showBaseline(base):
    for elem in showResultsOrder:
        print("Median "+ elem + " latency:" + str(round(base[elem],degree+1)))

def showBaselineResults(bases):
    i = 0
    for baseLine in bases:
        print("Baseline: " + baselines[i])
        i = i+1
        showBaseline(baseLine)
        print("=====================")

def showBase(bench, base):
    for elem in showResultsOrder:
        print("Median "+ elem + " latency:" + str(round(bench[elem],degree+1))+ " | Overhead:"+ str(round((bench[elem]/base[elem] - 1)*100,degree+1))+"%")

def showBenchmarkResults(benches, bases):
    i = 0
    for base in bases:
        print("")
        print("Comparing against baseline: " + baselines[i])
        print("")
        print("")
        i = i+1
        j = 0
        for bench in benches:
            print("============sof bench===========")
            print("Benchmark:" + benchmarks[j])
            j = j+1
            showBase(bench, base)
            print("")
            print("Geometric Mean Overhead:" + str(round(computeGeometricMean(bench,base),degree)) + "%")
            print("============eof bench===========")
            print("")
        

        print("======eof baseline=======")

def generateLatexHeaderOnePGOBase(numBenches, numBases):
   numLs = (2)*(numBenches-1)+ numBases + 2
   latexString = "|"
   for i in range(0,numLs):
       latexString = latexString+"l|"
   
   
   print("\\begin{tabular}{"+ latexString+ "}")
   print("\\hline")

   latexString = "Test" 

   for i in range(0,numBenches-1):
         latexString = latexString + "&\\multicolumn{2}{|l|}{PlaceHolder}"

   latexString = latexString + "&\\multicolumn{"+ str(numBases+1)+"}{|l|}{PlaceHolder}"
   
   print(latexString+"\\\\ \\hline")

def computeGeometricMean(bench, base):
   geoVec = []
   for elem in benchSelectionSablon:
       geoVec.append(bench[elem]/base[elem])
   return (geo_mean(geoVec) - 1)*100

def outputLatexTableContentsOnePGOBase(benches, bases):
   latexString = ""
   generateLatexHeaderOnePGOBase(len(benches), len(bases))
   for bench in benchSelectionSablon:
       latexString = bench.replace("_","\\_")
       for measurement in benches:
           latexString = latexString+"&"+ str(round(measurement[bench],degree))
           latexString = latexString+"&"+ "\\textbf{"+str(round(measurement[bench]/bases[0][bench],degree))+"x}"
       for overhead in bases[1:]:
               latexString = latexString+"&"+ "\\textbf{"+str(round(measurement[bench]/overhead[bench],degree))+"x}"
       latexString = latexString+"\\\\ \\hline"

       print(latexString)
   latexString = "Geometric Mean"
   for bench in benches:
        latexString = latexString +"&-&\\textbf{"+str(round(computeGeometricMean(bench,bases[0]),degree)) + "\\%}"

   for base in bases[1:]:
        latexString = latexString+"&\\textbf{"+str(round(computeGeometricMean(bench,base),degree)) + "\\%}"
   latexString = latexString+"\\\\ \\hline"
   
   print(latexString)

   print("\\end{tabular}")

def generateLatexHeaderJustOverhead(size):
   numLs = 1+size
   latexString = "|"
   for i in range(0,numLs):
       if i == 0:
          latexString = latexString+"l|"
       else:
          latexString = latexString+"r|"
   
   print("\\begin{tabular}{"+ latexString+ "}")
   print("\\hline")

   latexString = "\\multicolumn{1}{|l|}{Test}" 

   for i in range(0,size):
         latexString = latexString + "&\\multicolumn{1}{|l|}{PlaceHolder}"
   print(latexString+"\\\\ \\hline")

def generateLatexHeaderJustOverhead2(size, sablon):
   numLs = 1+size
   latexString = "|"
   for i in range(0,numLs):
       if i == 0:
          latexString = latexString+"l|"
       else:
          latexString = latexString+"r|"
   
   print("\\begin{tabular}{"+ latexString+ "}")
   print("\\hline")

   latexString = "\\multicolumn{1}{|l|}{Test}" 

   for i in range(0,size):
         latexString = latexString + "&\\multicolumn{1}{|l|}{"+ sablon[i]+"}"
   print(latexString+"\\\\ \\hline")

def outputLatexTableJustOverhead(benches, bases, sablon):
   latexString = ""
   generateLatexHeaderJustOverhead2(len(benches), sablon)
   for bench in benchSelectionSablon:
       latexString = bench.replace("_","\\_")
       for measurement in benches:
           latexString = latexString+"&"+ "\\textbf{"+str(round((measurement[bench]/bases[0][bench] - 1)*100,degree))+"\\%}"
       latexString = latexString+"\\\\ \\hline"

       print(latexString)
   latexString = "Geometric Mean"
   for bench in benches:
        latexString = latexString +"&\\textbf{"+str(round(computeGeometricMean(bench,bases[0]),degree)) + "\\%}"
   latexString = latexString+"\\\\ \\hline"
   
   print(latexString)

   print("\\end{tabular}")

def outpuLatexTableJustBaselines(bases):
   latexString = ""
   instance = 0
   generateLatexHeaderJustOverhead(len(bases), tablesablon)
   for bench in benchSelectionSablon:
       latexString = bench.replace("_","\\_")
       latexString = latexString+"&"+ str(round(bases[0][bench],degree+1))
       for measurement in bases[1:]:       
           latexString = latexString+"&"+ str(round(measurement[bench],degree+1)) + "&\\textbf{"+str(round((measurement[bench]/bases[0][bench] - 1)*100,degree))+"\\%}"
       latexString = latexString+"\\\\ \\hline"

       print(latexString)
   latexString = "Geometric Mean&-"
   for bench in bases[1:]:
        latexString = latexString +"&" + "\\multicolumn{2}{|r|}{\\textbf{"+str(round(computeGeometricMean(bench,bases[0]),degree)) + "\\%}}"
   latexString = latexString+"\\\\ \\hline"
   
   print(latexString)

   print("\\end{tabular}")
   

def generateLatexHeader(numBenches, numBases):
   numLs = (numBases+1)*numBenches+1
   latexString = "|"
   for i in range(0,numLs):
       latexString = latexString+"l|"
   
   
   print("\\begin{tabular}{"+ latexString+ "}")
   print("\\hline")

   latexString = "Test" 

   for i in range(0,numBenches):
         latexString = latexString + "&\\multicolumn{"+ str(numBases+1)+"}{|l|}{PlaceHolder}"
   
   print(latexString+"\\\\ \\hline")

def outputLatexTableContentsNoBases(benches, bases):
   latexString = ""
   generateLatexHeader(len(benches), len(bases))
   for bench in benchSelectionSablon:
       latexString = bench.replace("_","\\_")
       for measurement in benches:
           latexString = latexString+"&"+ str(round(measurement[bench],degree))
           for overhead in bases:
               latexString = latexString+"&"+ "\\textbf{"+str(round(measurement[bench]/overhead[bench],degree))+"x}"
       latexString = latexString+"\\\\ \\hline"

       print(latexString)

   print("\\end{tabular}")


def parseFile(name, instanceMap):
    iterate = False
    iterate_ctx = False
    iterate_ctxPipe = False
    mapCounter = 0
    ctx_Counter = 0
    inputFile =  open(name,encoding="utf8", errors='ignore')
    for line in inputFile:
        if 'Simple syscall:' in line:
            #print float(line.split(':')[1].split(" ")[1]
            instanceMap["null"].append(float(line.split(':')[1].split(" ")[1]))
        if 'Simple open/close:' in line:
            instanceMap["open"].append(float(line.split(':')[1].split(" ")[1]))
        if 'Simple fstat:' in line:
            instanceMap["fstat"].append(float(line.split(':')[1].split(" ")[1]))
        if 'Process fork+exit:' in line:
            instanceMap["fork/exit"].append(float(line.split(':')[1].split(" ")[1]))
        if 'Process fork+execve:' in line:
            instanceMap["fork/exec"].append(float(line.split(':')[1].split(" ")[1]))
        if 'Process fork+/bin/sh -c:' in line:
            instanceMap["fork/shell"].append(float(line.split(':')[1].split(" ")[1]))
        if 'Pipe latency:' in line:
            instanceMap["pipe"].append(float(line.split(':')[1].split(" ")[1]))
        if 'Select on 500 fd\'s:' in line:
            instanceMap["select_file"].append(float(line.split(':')[1].split(" ")[1]))
        if 'Select on 500 tcp fd\'s:' in line:
            instanceMap["select_tcp"].append(float(line.split(':')[1].split(" ")[1]))
        if 'Pagefaults on' in line:
            instanceMap["page_fault"].append(float(line.split(':')[1].split(" ")[1]))
        if 'UDP latency using localhost:' in line:
            instanceMap["udp"].append(float(line.split(':')[1].split(" ")[1]))
        if 'TCP latency using localhost:' in line:
            instanceMap["tcp"].append(float(line.split(':')[1].split(" ")[1]))
        if 'AF_UNIX sock stream latency:' in line:
            instanceMap["af_unix"].append(float(line.split(':')[1].split(" ")[1]))
        if 'Signal handler installation:' in line:
            instanceMap["sig_install"].append(float(line.split(':')[1].split(" ")[1]))
        if 'Signal handler overhead:' in line:
            instanceMap["sig_dispatch"].append(float(line.split(':')[1].split(" ")[1]))
        if 'TCP/IP connection cost to localhost:' in line:
            instanceMap["tcp_conn"].append(float(line.split(':')[1].split(" ")[1]))
        if 'Simple read:' in line:
            instanceMap["read"].append(float(line.split(':')[1].split(" ")[1]))
        if 'Simple write:' in line:
            instanceMap["write"].append(float(line.split(':')[1].split(" ")[1]))
        if 'Simple stat:' in line:
            instanceMap["stat"].append(float(line.split(':')[1].split(" ")[1]))
        
        if iterate:
             mapCounter +=1
             if mapCounter == 2:
                 instanceMap["mmap"].append(float(line.split(" ")[1]))
                 iterate = False
        if iterate_ctx:
             ctx_Counter +=1
             if ctx_Counter == 1:
                 instanceMap["ctx_4k_pipe"].append(float(line.split(" ")[1]))
                 iterate_ctx = False
        if iterate_ctxPipe:
             ctx_Counter +=1
             if ctx_Counter == 3:
                 instanceMap["ctx_64k_pipe"].append(float(line.split(" ")[1]))
                 iterate_ctxPipe = False
        if '\"mappings' in line:
             iterate = True
             mapCounter = 0
    #    if 'size=4k ovr=' in line:
    #         ctxlatency.append(float(line.split("ovr=")[1]))
    #    if 'size=64k ovr=' in line:
    #         ctxlatencyPipe.append(float(line.split("ovr=")[1]))
        if 'size=4k ovr=' in line:
             iterate_ctx = True
             ctx_Counter = 0
        if 'size=64k ovr=' in line:
             iterate_ctxPipe = True
             ctx_Counter = 0
    inputFile.close()

def parseFolder(folder, instance, resultsVector):
    resultsVector.append(copy.deepcopy(resultsSablon))
    for i in range(0 , numProfiles):
          parseFile('./'+folder+'/'+base+str(i), resultsVector[instance])

    #sortResultTable(resultsVector[instance])
    



def main():
    global baselines, benchmarks, benchSelectionSablon, showResultsOrder, tablesablon
    trigger = 0
    if len(sys.argv) != 1:
       baselines = []
       benchmarks = []
       for arg in sys.argv[1:]:
           if arg == "-all":
               baselines = allbaselines
               benchmarks = allbenchmarks
           if arg == "-only-retpolines":
               benchSelectionSablon = retpolinesSelectionSablon  
               showResultsOrder =  retpolinesSelectionSablon
               baselines=   [ 'baseline.noopt']
               benchmarks = ['+retpolines.noopt' , '+retpolines.pibe-opt.lmbench-work.999990']
               break
           if arg == "-only-retpolines-test":
               benchSelectionSablon = retpolinesSelectionSablon  
               showResultsOrder =  retpolinesSelectionSablon
               baselines=   [ 'test.baseline.noopt']
               benchmarks = ['test.+retpolines.noopt' , 'test.+retpolines.pibe-opt.lmbench-work.999990']
               break
           if arg == "-baselines":
               baselines=[]
               trigger = 1
               continue
           if arg == "-benchmarks":
               trigger = 2
               continue
           if arg == "-sablon":
               trigger = 3
               continue
           if trigger == 1:
               baselines.append(arg)
           if trigger == 2:
               benchmarks.append(arg)
           if trigger == 3:
               tablesablon.append(arg)
    instance = 0;
    resultsVector = []
    for folder in benchmarks:
        parseFolder(folder, instance, resultsVector)
        instance = instance + 1
    instance = 0;
    baselineVector = []
    for folder in baselines:
        parseFolder(folder, instance, baselineVector)
        instance = instance + 1

    meanResultsTable = computeMeanInstances(resultsVector)
    meanBaselineTable = computeMeanInstances(baselineVector)
    outputLatexTableJustOverhead(meanResultsTable, meanBaselineTable, tablesablon)
    

main()
