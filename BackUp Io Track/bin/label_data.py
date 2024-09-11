import pandas as pd
import numpy as np
from sklearn.model_selection import train_test_split
from sklearn.svm import SVC
from pickle import dump

dataset = pd.read_csv("/home/pizero/bin/captured_data.csv")

#Se dividen los datos para procesarlos con tranformada rapida de fourier
fT1 = np.abs(np.fft.fft(dataset.iloc[:,:1000]))
fT2 = np.abs(np.fft.fft(dataset.iloc[:,1000:]))

df2 = fT1/fT2


l=[]
l.extend(range(0,len(df2[0])))

header = np.array(l)
reading = np.stack((header,df2[0]))

read = pd.DataFrame(reading)
read['class'] = 0#flojo #1#firme

read.to_csv("/home/pizero/bin/captured_data.csv", index=False) 
