import pandas as pd
import pickle
from os.path import join, dirname
import subprocess 
from gpiozero import LED
from time import sleep
import numpy as np

from os import remove

from configparser import ConfigParser
input_config = ConfigParser()
config_path = join(dirname(__file__), 'config.ini')
input_config.read(config_path)

model_filename = input_config['DEFAULT']['model_filename']
led = LED(int(input_config['DEFAULT']['LED']))
duration = int(input_config['DEFAULT']['duration'])
c_file = input_config['DEFAULT']['c_file']
input_file = input_config['DEFAULT']['input_file']

loaded_model = pickle.load(open(model_filename, 'rb'))

delay=0.5

led.on()
sleep(duration-2)
led.off()

while True:
	try:
		subprocess.run(["sudo", c_file])
		
		df = pd.read_csv(input_file)
		
		print(len(df))

		fT1 = np.abs(np.fft.fft(df.iloc[:,:1000]))
		fT2 = np.abs(np.fft.fft(df.iloc[:,1000:]))

		df2 = fT1/fT2

		
		l=[]
		l.extend(range(0,len(df2[0])))

		header = np.array(l)
		reading = np.stack((header,df2[0]))


		X_test = pd.DataFrame(reading)
		y_pred = loaded_model.predict(X_test)

		print(y_pred)

		if y_pred[-1] == 0:
			led.on()
			sleep(delay)
			led.off()
			sleep(delay)
			led.on()
			sleep(delay)
			led.off()
			sleep(delay)
			led.on()
			sleep(delay)
			led.off()
			sleep(delay)
			led.on()
			sleep(delay)
			led.off()
			sleep(delay)
			led.on()
			sleep(delay)
			led.off()
		else:
			led.on()
			sleep(duration)
			led.off()

		
		remove(input_file) 
	except KeyboardInterrupt:
		break
	except:
		pass
	
