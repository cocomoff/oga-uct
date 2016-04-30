import matplotlib.pyplot as plt
from sys import argv 
import numpy as np

domain = argv[1]

base = np.genfromtxt('RESULTS_' + domain + '/ALL/baseNew.txt', delimiter=' ')
asap = np.genfromtxt('RESULTS_' + domain + '/ALL/asapNew.txt', delimiter=' ')
k1 = np.genfromtxt('RESULTS_' + domain + '/ALL/k1New.txt', delimiter=' ')
k2 = np.genfromtxt('RESULTS_' + domain + '/ALL/k2New.txt', delimiter=' ')
k3 = np.genfromtxt('RESULTS_' + domain + '/ALL/k3New.txt', delimiter=' ')
        
# plt.plot(base[:,0], base[:,1], label = "Vanilla") #'b-',
# plt.plot(asap[:,0], asap[:,1], 'r-', label = "ASAP")
# plt.plot(k1[:,0], k1[:,1], 'g-', label = "K=1")
# plt.plot(k3[:,0], k3[:,1], """'b-',""" label = "ASAP")

#plt.subplot(111, axisbg='darkslategray')
plt.gcf().set_facecolor('lightgrey')

plt.xlabel('Time (sec)',fontsize=20, labelpad = 20)
plt.ylabel('Accumulated Cost',fontsize=20, labelpad = 20)#'large')

plt.errorbar(base[:,3], base[:,1], yerr = 2*base[:,2], color='blue', linewidth = 2, label = "Vanilla")
plt.errorbar(asap[:,3], asap[:,1], yerr = 2*asap[:,2], color='red', linewidth = 2, label = "ASAP")
plt.errorbar(k1[:,3], k1[:,1], yerr = 2*k1[:,2], color='goldenrod', linewidth = 2, label = "K=1")
plt.errorbar(k2[:,3], k2[:,1], yerr = 2*k2[:,2], color='violet', linewidth = 2, label = "K=2")
plt.errorbar(k3[:,3], k3[:,1], yerr = 2*k3[:,2], color='green', linewidth = 2, label = "K=3")

plt.legend(prop={'size':17})	# loc = 5, 

plt.tick_params(axis='x',labelsize=16)
plt.tick_params(axis='y',labelsize=16)

plt.title(domain + ' with time', fontsize = 30, y=1.02)#sys.argv[4])
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
