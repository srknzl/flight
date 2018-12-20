import sys
if(len(sys.argv) != 2):
  print("give 1 arg")
  sys.exit(1)
numOfSeats = int(sys.argv[1])
array = []
f=open("output.txt","r")
for i in f.read().split():
  array.append(int(i))
f.close()
answer = []
for i in range(numOfSeats):
  answer.append(i+1)
if(sorted(array) == answer):
  print("ok")
