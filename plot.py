import matplotlib.pyplot as plt
import re


rangeN = [256, 512, 1024, 2048, 4096]
rangeB = [8, 16]
rangeP = [0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0]


if __name__ == "__main__":
	allResults = {}
	for itemx in rangeN:
		allResults[str(itemx)]={}
		for itemy in rangeB:
			allResults[str(itemx)][str(itemy)] = []
	
	p = re.compile("(\d+),(\d+),(\d+\.\d*),(\d+\.\d*),(\d+\.\d*),(\d+\.\d*)")
							    
	infile = open("results.txt", "r")
	for line in infile:
		m = p.match(line)
		if m:
			allResults[str(m.groups()[0])][str(m.groups()[1])].append(float(m.groups()[5]))
	for itemx in allResults.keys():
		line, = plt.plot(rangeP, allResults[itemx]['16'], '--', linewidth=2)

	plt.show()

														        
															        
																    
																        

