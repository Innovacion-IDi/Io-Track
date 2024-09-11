import pandas as pd
import numpy as np
from sklearn.model_selection import train_test_split
from sklearn.svm import SVC
from pickle import dump

# Cargar el CSV con datos "buenos" y asignar la clase 0
buenos = pd.read_csv("/path/to/captured_data_bueno.csv")
buenos['class'] = 0  # Clase 0 = "buenos"

# Cargar el CSV con datos "malos" y asignar la clase 1
malos = pd.read_csv("/path/to/captured_data_malo.csv")
malos['class'] = 1  # Clase 1 = "malos"

# Combinar ambos conjuntos de datos en un solo DataFrame
data = pd.concat([buenos, malos])

# Dividir los datos en características (X) y etiquetas (y)
X = data.drop('class', axis=1)
y = data['class']

# Dividir los datos en entrenamiento y prueba (80% entrenamiento, 20% prueba)
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.20)

# Crear el clasificador de vectores de soporte (SVM)
svclassifier = SVC(kernel='linear')  # Puedes cambiar el kernel según tu preferencia
svclassifier.fit(X_train, y_train)

# Realizar predicciones en el conjunto de prueba
y_pred = svclassifier.predict(X_test)
print(y_pred)

# Guardar el modelo entrenado en un archivo
filename = '/path/to/ferro_tuercas_fourier.sav'
dump(svclassifier, open(filename, 'wb'))
