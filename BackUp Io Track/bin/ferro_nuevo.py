import pandas as pd
import pickle
import csv
from os.path import join, dirname
from subprocess import run
from gpiozero import LED
from time import sleep


from configparser import ConfigParser
input_config = ConfigParser()
config_path = join(dirname(__file__), 'config.ini')
input_config.read(config_path)

filename=input_config['DEFAULT']['filename']
new_file=input_config['DEFAULT']['new_file']
model_filename = input_config['DEFAULT']['model_filename']
led = LED(int(input_config['DEFAULT']['LED']))
duration = int(input_config['DEFAULT']['duration'])
c_file = input_config['DEFAULT']['c_file']

run(["sudo", c_file])

header=[]
lineas=[]

with open(filename,'r') as data:
    for line in csv.reader(data):
        res = [eval(i) for i in line]
        lineas.append(res)

header.extend(range(1,len(lineas[0])+1))

with open(new_file, 'w', encoding='UTF8') as f:
    writer = csv.writer(f)
    writer.writerow(header)
    writer.writerows(lineas)

dataframe = pd.read_csv(new_file)
loaded_model = pickle.load(open(model_filename, 'rb'))
y_pred = loaded_model.predict(dataframe)[0]

print(y_pred)

if y_pred == 0:
    led.on()
    sleep(5)
    led.off()
else:
    led.off()
