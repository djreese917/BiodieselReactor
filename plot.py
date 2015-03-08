#Stack overflow: 
#Real time matplotlib plot is not working while still in a loop

import serial
import matplotlib.pyplot as plt
import time
import random
from collections import deque
import numpy as np

class Plt:

	def __init__(self):
		self.fig = plt.Figure()
		self.ax = plt.axes(xlim=(0,1000), ylim=(0,200))
		self.a1 = deque([0]*1000)			
		self.line, = plt.plot(self.a1)
		plt.ion()
		plt.ylim([0,200])

	def Show(self):
		print "Showing!"
		plt.show() 
	
	def Close(self):
		plt.close('all')

	def Update(self, serial, show): #takes an string (MUST be a number string)
		data = serial.readline()
		try:
			numData = float(data)
		except ValueError:
                        print "Value error!"
			return False

                if show == False:
			return False

        	self.a1.appendleft(numData)
		self.datatoplot = self.a1.pop()
		self.line.set_ydata(self.a1)
		plt.draw()
		plt.pause(0.0001)         
	
		return True
		
