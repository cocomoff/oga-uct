import matplotlib.pyplot as plt
from sys import argv 
import numpy as np

domain = argv[1]

base = np.genfromtxt('RESULTS_' + domain + '/ALL/baseNew.txt', delimiter=' ')
asap = np.genfromtxt('RESULTS_' + domain + '/ALL/asapNew.txt', delimiter=' ')
k = np.genfromtxt('RESULTS_' + domain + '/ALL/k' + argv[2] + 'New.txt', delimiter=' ')
prunedk = np.genfromtxt('RESULTS_' + domain + '/ALPHA2/k' + argv[2] + 'New.txt', delimiter=' ')
#k1 = np.genfromtxt('RESULTS_' + domain + '/ALL/k3New.txt', delimiter=' ')
#k2 = np.genfromtxt('RESULTS_' + domain + '/ALL/k2New.txt', delimiter=' ')
#k3 = np.genfromtxt('RESULTS_' + domain + '/ALL/k3New.txt', delimiter=' ')
        
# plt.plot(base[:,0], base[:,1], label = "Vanilla") #'b-',
# plt.plot(asap[:,0], asap[:,1], 'r-', label = "ASAP")
# plt.plot(k1[:,0], k1[:,1], 'g-', label = "K=1")
# plt.plot(k3[:,0], k3[:,1], """'b-',""" label = "ASAP")

plt.xlabel('Time(msec)',fontsize=50, labelpad = -2)
plt.ylabel('Accumulated Cost',fontsize=50, labelpad = 0)#'large')


plt.errorbar(base[:,3], base[:,1], yerr = 2*base[:,2], color='blue', linewidth = 10, label = "UCT")
plt.errorbar(asap[:,3], asap[:,1], yerr = 2*asap[:,2], color='red', linewidth = 10, label = "ASAP-UCT")
plt.errorbar(k[:,3], k[:,1], yerr = 2*k[:,2], color='black', linewidth = 10, label = "OGA K=" + argv[2])
plt.errorbar(prunedk[:,3], prunedk[:,1], yerr = 2*prunedk[:,2], color='black',linestyle='dashed', linewidth = 10, label = "Pruned OGA K=" + argv[2])
#plt.errorbar(k1[:,3], k1[:,1], yerr = 2*k1[:,2], color='black', linewidth = 10, label = "OGA-UCT")
#plt.errorbar(k2[:,3], k2[:,1], yerr = 2*k2[:,2], color='violet', linewidth = 2, label = "K=2")
#plt.errorbar(k3[:,3], k3[:,1], yerr = 2*k3[:,2], color='green', linewidth = 4, label = "OGA_UCT,K=3")

plt.legend(loc=0,prop={'size':60})	# loc = 5, 

plt.tick_params(axis='x',labelsize=50)
plt.tick_params(axis='y',labelsize=50)

if(len(argv) > 3):
	plt.title(argv[3], fontsize = 60, y=1.02)#sys.argv[4])
else:
	plt.title(domain, fontsize = 60, y=1.02)#sys.argv[4])

# plt.savefig('Graphs/' + nodes + '.png')
plt.show()

exit(0)

# Colors
# b: blue
# g: green
# r: red
# c: cyan
# m: magenta
# y: yellow
# k: black
# w: white
