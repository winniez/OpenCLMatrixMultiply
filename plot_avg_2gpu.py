import matplotlib.pyplot as plt
import re


rangeN = [256, 512, 1024, 2048, 2560, 3072, 3584, 4096]
rangeB = [8, 16]
rangeP = [0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0]
rangeD = ['480', '680'] 


if __name__ == "__main__":
    allResults = {}
    for itemx in rangeN:
        allResults[str(itemx)]={}
        for itemy in rangeB:
            allResults[str(itemx)][str(itemy)] = {}
            for itemz in rangeD:
                    allResults[str(itemx)][str(itemy)][itemz] = []
    #print allResults.keys()
    #print allResults['256'].keys()
    #print allResults['256']['8']
    p = re.compile("(\d+),(\d+),(\d+),(\d+\.\d*),(\d+\.\d*),(\d+\.\d*),(\d+\.\d*),(\d+\.\d*)")
    
    infile = open("results_480.txt", "r")
    for line in infile:
        m = p.match(line)
        if m:
            allResults[str(m.groups()[0])][str(m.groups()[1])]['480'].append(float(m.groups()[7]))

    infile.close()

    infile = open("results_680.txt", "r")
    for line in infile:
        m = p.match(line)
        if m:
            allResults[str(m.groups()[0])][str(m.groups()[1])]['680'].append(float(m.groups()[7]))

    infile.close()

    raw480 = allResults["4096"]["16"]["480"]
    raw680 = allResults["4096"]["16"]["680"]
    print len(raw480)
    print len(raw680)
    print len(rangeP)
    avg480 = []
    avg680 = []

    for i in range (0, len(rangeP)):
        for j in range (0, 2):
            if j == 0:
                avg480.append(raw480[i*3+j])
            else:
                avg480[i] = raw480[i*3+j] + avg480[i]
            if j == 2:
                avg480[i] = avg480[i]/3

    for i in range (0, len(rangeP)):
        for j in range (0, 2):
            if j == 0:
                avg680.append(raw680[i*3+j])
            else:
                avg680[i] = raw680[i*3+j] + avg680[i]
            if j == 2:
                avg680[i] = avg680[i]/3

    print len(avg680)            
    ratio = []

    for i in range (0, len(rangeP)):
        ratio.append(avg480[i] / avg680[i])

    print len(ratio)

    for itemx in allResults.keys():
        line, = plt.plot(rangeP, ratio, '--', linewidth=2)

    plt.title("480 running time over 680")
    plt.xlabel("percentage")
    plt.ylabel("speed up")

    plt.show()
