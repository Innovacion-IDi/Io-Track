import pandas as pd
import cv2
import numpy as np
import os
import time

# Ruta del CSV proporcionada
csv_path = "/home/pizero/bin/captured_data.csv"

# Configuración de la imagen
height = 400  # Altura de la imagen
width = 800   # Anchura de la imagen

# Crear una función para graficar la onda
def graficar_onda():
    try:
        # Cargar los datos del CSV
        data = pd.read_csv(csv_path, header=None)
        
        # Los valores de la señal
        signal = data.values.flatten()

        # Generar una serie de tiempo basada en la cantidad de muestras
        time = np.arange(len(signal))

        # Crear una imagen en blanco
        image = np.ones((height, width, 3), dtype=np.uint8) * 255

        # Normalizar los valores de la señal a la altura de la imagen
        signal_normalized = np.interp(signal, (min(signal), max(signal)), (height, 0))

        # Escalar el tiempo a la anchura de la imagen
        time_normalized = np.interp(time, (min(time), max(time)), (0, width))

        # Dibujar la onda en la imagen
        for i in range(1, len(signal_normalized)):
            start_point = (int(time_normalized[i-1]), int(signal_normalized[i-1]))
            end_point = (int(time_normalized[i]), int(signal_normalized[i]))
            cv2.line(image, start_point, end_point, (0, 0, 255), 2)  # Línea roja

        # Mostrar la imagen con la onda graficada
        cv2.imshow('Onda Actualizada', image)
        if cv2.waitKey(1) & 0xFF == ord('q'):  # Presionar 'q' para salir
            return False
        return True
    except Exception as e:
        print(f"Error al graficar la onda: {e}")
        return True

# Monitorear cambios en el tamaño del archivo CSV
def actualizar_grafica_continuamente():
    while True:
        # Graficar continuamente
        if not graficar_onda():
            break  # Salir del bucle si el usuario presiona 'q'
        
        # Dormir por un corto periodo antes de volver a graficar
        time.sleep(0.1)  # Ajusta este tiempo según tus necesidades

# Iniciar la actualización continua del gráfico
try:
    actualizar_grafica_continuamente()
finally:
    # Asegurarse de cerrar la ventana cuando el script finaliza
    cv2.destroyAllWindows()
