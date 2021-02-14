import fileinput
for line in fileinput.input():
    if "deep LBR" in line:
         prior = ""
         for items in line.replace(",", "").split():
             if (items == "LBR"):
                 print(prior.split("-")[0])
             prior = items
              
