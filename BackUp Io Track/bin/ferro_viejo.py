import pandas as pd
import pickle
import csv


filename="/home/pizero/Documents/MCP3008/captured_data.csv"

lineas=[]

with open(filename,'r') as data:
    for line in csv.reader(data):
        res = [eval(i) for i in line]
        suma=1 if sum(res) > 0 else 0
        res.append(suma)
        lineas.append(res)

print(len(lineas[0]))


header=[]
header.extend(range(1,len(lineas[0])))
header.append("class")

with open('/home/pizero/bin/captured_data_ex.csv', 'w', encoding='UTF8') as f:
    writer = csv.writer(f)
    writer.writerow(header)
    writer.writerows(lineas)

dataframe = pd.read_csv("/home/pizero/bin/captured_data_ex.csv")
filename = '/home/pizero/bin/ferro_tuercas_v2.sav'
#filename = '/home/pizero/bin/finalized_model_sigmoid_83.sav'
loaded_model = pickle.load(open(filename, 'rb'))
XT = dataframe.drop('class', axis=1)
y_pred = loaded_model.predict(XT)[0]
print(y_pred)
