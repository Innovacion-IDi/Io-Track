from os.path import join, dirname
import subprocess 
from gpiozero import LED
from time import sleep

from configparser import ConfigParser
input_config = ConfigParser()
config_path = join(dirname(__file__), 'config.ini')
input_config.read(config_path)


led = LED(int(input_config['DEFAULT']['LED']))
duration = int(input_config['DEFAULT']['duration'])
c_file = input_config['DEFAULT']['c_file']


led.on()
sleep(duration-2)
led.off()

while True:
	try:
		subprocess.run(["sudo", c_file])

	except KeyboardInterrupt:
		break
	except:
		pass
