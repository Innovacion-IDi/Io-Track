import pandas as pd
import pickle
#import csv
from os.path import join, dirname
import subprocess 
from gpiozero import LED
from time import sleep
import numpy as np

import psycopg2
from datetime import datetime

from configparser import ConfigParser
input_config = ConfigParser()
config_path = join(dirname(__file__), 'config.ini')
input_config.read(config_path)

model_filename = input_config['DEFAULT']['model_filename']
led = LED(int(input_config['DEFAULT']['LED']))
duration = int(input_config['DEFAULT']['duration'])
c_file = input_config['DEFAULT']['c_file']
input_file = input_config['DEFAULT']['input_file']
class_value = int(input_config['DEFAULT']['class_value'])

loaded_model = pickle.load(open(model_filename, 'rb'))

led.on()
sleep(5)
led.off()

conn = psycopg2.connect("host=localhost dbname=postgres user=postgres")
cur = conn.cursor()
cur.execute("""
    CREATE TABLE trama(
    id integer PRIMARY KEY NOT NULL,
    fecha-hora timestamp,
    
    datos float[],
    clase integer
)
""")
conn.commit()

# while True:
    # try:
        # subprocess.run(["sudo", c_file])

        # df = pd.read_csv(input_file)

        # fT1 = np.abs(np.fft.fft(df.iloc[:,:1000]))
        # fT2 = np.abs(np.fft.fft(df.iloc[:,1000:]))

        # df2 = fT1/fT2
        
        # current_date_time = datetime.now()

        # insert_query = "INSERT INTO trama VALUES {}, {}, {}".format(current_date_time,df2[0],class_value)
        # cur.execute(insert_query)
        # conn.commit()


        # from os import remove
        # remove(input_file) 
    # except KeyboardInterrupt:
        # break

subprocess.run(["sudo", c_file])

df = pd.read_csv(input_file)

fT1 = np.abs(np.fft.fft(df.iloc[:,:1000]))
fT2 = np.abs(np.fft.fft(df.iloc[:,1000:]))

df2 = fT1/fT2

current_date_time = datetime.now()

insert_query = "INSERT INTO trama VALUES {}, {}, {}".format(current_date_time,df2[0],class_value)
cur.execute(insert_query)
conn.commit()
