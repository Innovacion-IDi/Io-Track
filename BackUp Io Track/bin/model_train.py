import pandas as pd
import numpy as np
from sklearn.model_selection import train_test_split
from sklearn.svm import SVC
from pickle import dump

#dataset = pd.read_csv("/home/pizero/bin/captured_data.csv")

#Se dividen los datos para procesarlos con tranformada rapida de fourier
#fT1 = np.abs(np.fft.fft(dataset.iloc[:,:1000]))
#fT2 = np.abs(np.fft.fft(dataset.iloc[:,1000:]))

#df2 = fT1/fT2


#l=[]
#l.extend(range(0,len(df2[0])))

#header = np.array(l)
#reading = np.stack((header,df2[0]))

#read = pd.DataFrame(reading)
#read['class'] = 0#flojo #1#firme


read = pd.read_csv("/home/pizero/bin/captured_data.csv")

X = read.drop('class', axis=1)
y = read['class']


X_train, X_test, y_train, y_test = train_test_split(X, y, test_size = 0.20)


#Clasificador de vectores de soporte
svclassifier = SVC(kernel='linear')#{'linear', 'poly', 'rbf', 'sigmoid', 'precomputed'}
svclassifier.fit(X_train, y_train)

#Prediccion de prueba del modelo
y_pred = svclassifier.predict(X_test)
print(y_pred)

#Se guarda el modelo entrenado
filename = '/home/pizero/bin/ferro_tuercas_fourier.sav'
dump(svclassifier, open(filename, 'wb'))
