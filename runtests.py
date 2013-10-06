import os, subprocess, re
import sys

execCmd = "./matrixmultiplication "

compileCmd = "gcc -DNSIZE=%d -DBLOCKSIZE=%d -DPERCENTAGE=%f -Wno-comment -std=gnu99 -lOpenCL -lm  matrixmultiplication.c -o matrixmultiplication"

def runtest(N, B, p, outfile):
	# This compiles the 
	os.system(compileCmd % (N, B, p))

	#p = re.compile(" INFO: Wall time for [a-zA-Z_]+ = ([0-9]+.?[0-9]*)")
	p = re.compile("(\d+),(\d+),(\d+\.\d*),(\d+\.\d*),(\d+\.\d*),(\d+\.\d*)")
	runtimes = []
	proc = subprocess.Popen(execCmd, shell=True, bufsize=256, stdout=subprocess.PIPE)
	for line in proc.stdout:
		print line
		#outfile.write(line)
		m = p.match(line)
		if m:
			outfile.write(line)
		#m = p.match(line)
		#if m:
		#	runtimes.append(float(m.group(1)))
		# print errors as we see them in the output
		#else:
		#	print line
	return runtimes

def printResults(allResults, filename="results.txt"):
	outfile = open(filename, "w")
	sortedKeys = allResults.keys()
	sortedKeys.sort()
	for key in sortedKeys:
		line.append("\t%.4f" % allResults[key])
		outfile.write("".join(line) + "\n")
	outfile.close()

if __name__=="__main__":
	if "-h" in sys.argv:
		print """usage: ./runtests.py """
		sys.exit(-1)
	rangeN = [256, 512, 1024, 2048, 4096]
	rangeB = [8, 16]
	rangeP = [0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0]
	allResults = {}
	filename="results.txt"
	outfile = open(filename, "w")
	for N in rangeN:
		for  B in rangeB:
			for P in rangeP:
				runtest(N, B, P, outfile)


	outfile.close()
