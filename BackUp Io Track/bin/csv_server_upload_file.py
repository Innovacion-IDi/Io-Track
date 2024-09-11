#url = 'http://89.116.214.114/api/csv/csv/'
#input_file='/home/Audacia/Documents/firmes_p1.csv'

#from os import remove
import schedule
#from json import loads, dump
from gpiozero import LED
from time import sleep
from datetime import datetime
from subprocess import run
from requests import post, get, codes
from os.path import join, dirname
from configparser import ConfigParser

input_config = ConfigParser()
config_path = join(dirname(__file__), 'config.ini')
input_config.read(config_path)


c_file = input_config['DEFAULT']['c_file']
input_file = input_config['DEFAULT']['input_file']
file_url = input_config['DEFAULT']['file_url']
led = LED(int(input_config['DEFAULT']['LED']))
duration = int(input_config['DEFAULT']['duration'])
on_duration = float(input_config['DEFAULT']['on_duration'])

#input_file = input_config['DEFAULT']['trial_file']



def upload_me():
    title = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    with open(input_file, 'r') as f:
      r = post(file_url, data={'titulo':title}, files={'archivo': f})
    
    remove(input_file) 
    print("File uploaded successfully")


	# output.append(loads(r.text))
    #output = []
    # #Saving response in output
    # with open('/home/Audacia/Documents/Response.json','w') as outfile:
	    # dump(output, outfile, indent=4)

def im_up():

    led.on()
    sleep(on_duration)
    led.off()
    led.on()
    sleep(on_duration)
    led.off()
    led.on()
    sleep(on_duration)
    led.off()
    led.on()
    sleep(on_duration)
    led.off()
    led.on()
    sleep(on_duration)
    led.off()
    led.on()
    sleep(on_duration)
    led.off()
    print("I'm on")
    sleep(1)


def im_online():
    while True:
        response = get('http://www.google.com')
        if response.status_code == codes.ok:
        
            led.on()
            sleep(duration)
            led.off()
            print("I'm online")
            sleep(1)
            break
        else:
          pass

im_up()

im_online()

schedule.every(2).hours.do(upload_me)
#schedule.every(2).minutes.do(upload_me)

#upload_me()

while True:
    run(["sudo", c_file])
    # Checks whether a scheduled task is pending to run or not
    schedule.run_pending()
    sleep(1)
