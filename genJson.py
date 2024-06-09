import os
import json

list = []
for filename in os.listdir(r'.'):
    if len(filename) >= 5 and filename[-5] != 'm' and filename[-4] == '.' and filename[-3] == 'b' and filename[-2] == 'i' and filename[-1] == 'n':
        list.append(filename)

print(json.dumps(list))
with open("version.json","w") as file:
    file.write(json.dumps(list))