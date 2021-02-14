import sys
import os
import re

import argparse

os.environ['KERNEL_VERSION'] = '5.1.0'
os.environ['DEFAULT_PLAYGROUND_PATH'] = '/projects/icp_kernel/playground/'
os.environ['DEFAULT_METADATA_DIR'] = os.environ['DEFAULT_PLAYGROUND_PATH']+ 'metadata/nonpreemptive/'
os.environ['DEFAULT_QUEMU_DIR'] = os.environ['DEFAULT_PLAYGROUND_PATH']+ 'metadata/nonpreemptive/'
os.environ['DEFAULT_SYSTEM_MAP'] = os.environ['DEFAULT_QUEMU_DIR'] +'System.map-'+ os.environ['KERNEL_VERSION']
os.environ['DEFAULT_KERNEL_DUMP'] =  os.environ['DEFAULT_QUEMU_DIR'] + 'vmlinux_dump'
os.environ['DEFAULT_COVERAGE_MAP'] = os.environ['DEFAULT_METADATA_DIR'] + 'Coverage.Map'
os.environ['DEFAULT_PROFILE_MAP'] = os.environ['DEFAULT_METADATA_DIR'] + 'Profile.Map'
os.environ['DEFAULT_OUTPUT_FILE'] = 'Decoded.Profiles'


ProfileMap = []
CoverageMap = []
SymbolTable = []
ObjectMap = []

coverageSize = 0
symbolSize = 0
objectSize = 0
profileSize = 0

OUTPUT_DIRECT = 0x2
OUTPUT_INDIRECT = 0x1

outputType = 0x3

def generateNewPathDefaults(metadata_dir):
  os.environ['DEFAULT_METADATA_DIR'] = os.environ['DEFAULT_PLAYGROUND_PATH']+ 'metadata/'+ metadata_dir +"/"
  os.environ['DEFAULT_QUEMU_DIR'] = os.environ['DEFAULT_PLAYGROUND_PATH']+ 'metadata/'+ metadata_dir +"/"
  os.environ['DEFAULT_SYSTEM_MAP'] = os.environ['DEFAULT_QUEMU_DIR'] +'System.map-'+ os.environ['KERNEL_VERSION']
  os.environ['DEFAULT_KERNEL_DUMP'] =  os.environ['DEFAULT_QUEMU_DIR'] + 'vmlinux_dump'
  os.environ['DEFAULT_COVERAGE_MAP'] = os.environ['DEFAULT_METADATA_DIR'] + 'Coverage.Map'
  os.environ['DEFAULT_PROFILE_MAP'] = os.environ['DEFAULT_METADATA_DIR'] + 'Profile.Map'
  os.environ['DEFAULT_OUTPUT_FILE'] = 'Decoded.Profiles'

def testPrintMap(_map):
  for elem in _map:
    print(elem)

def loadCoverageMap(filename):
  global coverageSize
  inputFile = open(filename)
  for line in inputFile:
      elements=line.strip().split("||||")
      index_1 = elements[1].index("@")
      index_2 = elements[1].index(")")
      index_p1 = elements[0].index("@")
      index_p2 = elements[0].index("(")
      # Entry has format (CallSiteTag, CallSiteParent, LLVMStringRepresentation, CoverageIndex) 
      touple = (elements[1][index_1+19:index_2].strip() , elements[0][index_p1+1:index_p2], elements[2].strip(), int(elements[3]))
      CoverageMap.append(touple) 
  coverageSize = len(CoverageMap)
  inputFile.close()

def loadSymbolTable(filename):
  global symbolSize
  inputFile = open(filename)
  for line in inputFile:
      elements = line.strip().split(" ")
      # Entry has format (SymbolAddress, SymbolType, SymbolName) 
      touple = (elements[0], elements[1], elements[2])
      SymbolTable.append(touple)
  symbolSize = len(SymbolTable)
  inputFile.close()

def getType(assemblyFormat):
    index= assemblyFormat.find('callq 0x')
    if (index >= 0):
        return 1
    else:
        return 0

def loadKernelObject(filename, instFilter):
  global objectSize
  inputFile = open(filename)
  for line in inputFile:
      elements = line.replace("\t"," ").strip().split(":")
      if len(elements) >= 2:
         split_index = elements[1].find(instFilter)
         if split_index >= 0:
            call_signature_flat = elements[1][split_index:len(elements[1])]
            call_signature = re.sub("\s\s+" , " ", call_signature_flat)
            # Entry has format CSAddress, direct/indirect(0/1), AssemblyFormatOfCall(not used)
            touple = (elements[0], getType(call_signature), call_signature)
            ObjectMap.append(touple)
  objectSize = len(ObjectMap)
  inputFile.close()

def loadProfileData(filename):
  global profileSize
  inputFile = open(filename)
  for line in inputFile:
      elements=line.strip().split(" ")
      touple = (elements[0], int(elements[1]) , elements[2], int(elements[3]), elements[4])
      ProfileMap.append(touple)
  profileSize = len(ProfileMap)
  inputFile.close()

def getFunctionName(functionAddress):
  funcNames = list(filter(lambda x: x[0] == functionAddress, SymbolTable))
  return list(map(lambda x: x[2], funcNames))

def getParentRecursively(callsite, inferior, superior):
   if inferior == superior:
      return SymbolTable[superior][0]
   middle = int((inferior+superior)/2)
   touple_1 = SymbolTable[middle]
   touple_2 = SymbolTable[min(middle+1, symbolSize-1)]
   hexnumber_1 = int(touple_1[0], 16)
   hexnumber_2 = int(touple_2[0], 16)
   address = int(callsite, 16)
   if hexnumber_1 <= address and address < hexnumber_2:
      return touple_1[0]
   elif hexnumber_2 <= address:
      return getParentRecursively(callsite, min(middle+1, symbolSize-1), superior)
   elif address < hexnumber_1:
      return getParentRecursively(callsite, inferior, middle)

def getParentName(callsite):
  parentAddress = getParentRecursively(callsite, 0, symbolSize - 1 )
  parentList = list(filter(lambda x: x[0] == parentAddress, SymbolTable))
  return list(map(lambda x: x[2], parentList))

def getObjectSignatureRecursively(callsite, inferior, superior):
  if inferior == superior:
      return ObjectMap[superior]
  middle = int((inferior+superior)/2)
  hexnumber = int(ObjectMap[middle][0], 16)
  address = int(callsite, 16)
  if hexnumber == address:
     return ObjectMap[middle]
  elif hexnumber < address:
        return getObjectSignatureRecursively(callsite, min(middle+1, objectSize-1), superior)
  else:
        return getObjectSignatureRecursively(callsite, inferior, middle)

def getObjectSignature(callsite):
  return getObjectSignatureRecursively(callsite, 0, objectSize-1)

def getCoverageRecursive(tag, inferior, superior):
   if inferior == superior:
      return CoverageMap[superior]
   middle = int((inferior+superior)/2)
   intEntry = int(CoverageMap[middle][0])
   intTag = int(tag)
   if intEntry == intTag:
      return CoverageMap[middle]
   if intEntry < intTag:
      return getCoverageRecursive(tag, min(middle+1, coverageSize-1), superior)
   else:
      return getCoverageRecursive(tag, inferior, middle)

def getCoverageData(tag):
  return getCoverageRecursive(tag, 0, coverageSize-1)

def decodeProfile(profile):
   child = getFunctionName(profile[0])
   parent = getParentName(profile[2])
   callType = getObjectSignature(profile[2])
   coverage = getCoverageData(profile[4])

   if parent[0] == '__brk_limit':
      return (None, 'invalid parent', child , parent, callType[1], profile[3], coverage[3], coverage[2], coverage[1] )

   if coverage[1] not in parent:
      return (None, 'unknown parent', child , parent, callType[1], profile[3], coverage[3], coverage[2], coverage[1] )
   # Element format childAliases parentAliases callType executionCount, coverageId, llvmStringRepresentation
   return (child, parent, callType[1], profile[3], coverage[3], coverage[2] )

def decodeProfileMap(profileMap):
   return list(map(decodeProfile, profileMap))

def globalWeightAnalysis(profiles):
   profileWeight = 0
   callSiteMap = {}
   for profile in profiles:
       profileWeight += profile[3]
       callSiteMap[profile[0]] = profile[1]
   totalWeight = 0
   for elem in callSiteMap.values():
        totalWeight += elem
   print("Number of collected profiles: "+str(len(profiles)))
   print("Overall execution count(all arcs): "+ str(profileWeight))
   print("Brute execution count(all callsites): "+ str(totalWeight))

def typeFiltering(profiles, outputTy):
  iProfiles = []
  dProfiles = []
  if (outputTy & OUTPUT_INDIRECT):
    iProfiles = list(filter(lambda x: x[2] == 0, profiles))
  if (outputTy & OUTPUT_DIRECT):
    dProfiles =  list(filter(lambda x: x[2] == 1, profiles))
  return iProfiles+dProfiles

#Parser

def writeString(out, string):
    out.write(string)
  
def writeInt(out, integer):
    out.write(str(integer))

def writeStruct(out, elements):
    writeElement(out, elements[0])
    for element in elements[1:]:
        out.write("?")
        writeElement(out, element)

def writeTuple(out, elements):
    writeElement(out, elements[0])
    for element in elements[1:]:
        out.write("|")
        writeElement(out, element)

def writeElement(out, element):
    if type(element) == str:
       writeString(out, element)
       return
    if type(element) == int:
       writeInt(out, element)
       return
    if type(element) == list:
       writeStruct(out, element)
       return
    if type(element) == tuple:
       writeTuple(out, element)
       return 

def writeOutput(filename, profiles):
  out = open(filename, 'w')
  for profile in profiles:
        writeElement(out, profile)
        out.write("\n")
  out.close()

def parseArguments():
 parser = argparse.ArgumentParser(description='VUProfiler offline analysis.')
 parser.add_argument('-c','--coverage_map', nargs='?', 
                     const=os.environ['DEFAULT_COVERAGE_MAP'], 
                     default=os.environ['DEFAULT_COVERAGE_MAP'],
                     help='Path to the file used to load the coverage map')
 
 parser.add_argument('-s','--system_map', nargs='?', 
                     const=os.environ['DEFAULT_SYSTEM_MAP'], 
                     default=os.environ['DEFAULT_SYSTEM_MAP'],
                     help='Path to the file used to load the kernel System.map')

 parser.add_argument('-k','--kernel_dump', nargs='?', 
                     const=os.environ['DEFAULT_KERNEL_DUMP'], 
                     default=os.environ['DEFAULT_KERNEL_DUMP'],
                     help='Path to the file used to load the kernel object dump')

 parser.add_argument('-p','--profile_map', nargs='?', 
                     const=os.environ['DEFAULT_PROFILE_MAP'], 
                     default=os.environ['DEFAULT_PROFILE_MAP'],
                     help='Path to the file used to load the benchmarked profiles')

 parser.add_argument('--prune', nargs='?', 
                     const=None, 
                     default=None,
                     help='Prune until you have PROFILE_MAP% most expensive profiles')

 parser.add_argument('-o','--out', nargs='?', 
                     const=os.environ['DEFAULT_OUTPUT_FILE'] , 
                     default=os.environ['DEFAULT_OUTPUT_FILE'] ,
                     help='Prune until you have PROFILE_MAP% most expensive profiles')

 parser.add_argument('-f','--folder', nargs='?', 
                     const="" , 
                     default="" ,
                     help='Dummy argument.')

 #parser.add_argument()
 parser.add_argument('-i','--indirect', action='store_true', help="Output indirect call profiles")
 parser.add_argument('-d','--direct', action='store_true', help="Output direct call profiles")
 parser.add_argument('-t','--test', nargs='*', choices=['coverage-map', 'system-map', 'kernel-dump', 'profile-map'], help="Test profiler components")

 

 return parser.parse_args()


def main():
  global outputType
  print("Initializing profiler data structures...")
  if len(sys.argv) >= 3 and (sys.argv[1] == "-f" or sys.argv[1] == "--folder") :
     generateNewPathDefaults(sys.argv[2])

 

  paramList = parseArguments()
  
  if paramList.test != None and len(paramList.test) != 0:
      print('Executing test framework')
      for test in paramList.test:
          if (test == 'coverage-map'):
             loadCoverageMap(paramList.coverage_map)
             testPrintMap(CoverageMap)
             print("Size of CoverageMap is "+str(coverageSize))
          if (test == 'system-map'):
             loadSymbolTable(paramList.system_map)
             testPrintMap(SymbolTable)
          if (test == 'kernel-dump'):
             loadKernelObject(paramList.kernel_dump,'call')
             testPrintMap(ObjectMap)
          if (test == 'profile-map'):
             loadProfileData(paramList.profile_map)
             testPrintMap(ProfileMap)
             print("Size of ProfileMap is "+str(profileSize))
      return 

  print(paramList)
  print("Loading Coverage Map...")
  loadCoverageMap(paramList.coverage_map)

  print("Loading Symbol Table...")        
  loadSymbolTable(paramList.system_map)
  
  print("Loading Object Dump(direct/indirect callsite information)...")
  loadKernelObject(paramList.kernel_dump,'call')

  print("Loading profiling candidates...")
  loadProfileData(paramList.profile_map)  

  print("Order profile map based on execution count(descending)...")
  ProfileMap.sort(key= lambda tup: -tup[3])

  print("Global Weight analysis...")
  globalWeightAnalysis(ProfileMap)

  DecodedMap = []
  DecodedMap = decodeProfileMap(ProfileMap)
  
  falsePositives = list(filter(lambda x: x[0] == None, DecodedMap))
  print("Eliminated "+str(len(falsePositives))+" false positives...")

  #for elem in falsePositives:
  #    print(elem)
   
  DecodedMap = list(filter(lambda x: x[0] != None, DecodedMap))

  if (paramList.indirect == True):
     if (outputType == 0x3):
        outputType = OUTPUT_INDIRECT
  if (paramList.direct == True):
     if (outputType == 0x3):
        outputType = OUTPUT_DIRECT
     else:
        outputType += OUTPUT_DIRECT

  DecodedMap = typeFiltering(DecodedMap, outputType)
  DecodedMap.sort(key= lambda tup: -tup[3])

  pruningInterval = len(DecodedMap)
  if (paramList.prune != None):
     print("TODO must implement pruning(modifies pruningInterval)")
  
  #testPrintMap(DecodedMap)
  print('Writing output to file '+paramList.out+'...')
  writeOutput(os.environ['DEFAULT_METADATA_DIR']+paramList.out, DecodedMap[0: pruningInterval])
 

  #print(paramList)

main()
