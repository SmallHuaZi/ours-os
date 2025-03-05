import json

# 读取compile_commands.json文件
print("Load... file")
with open('target/compile_commands.json', 'r') as file:
    data = json.load(file)

# 替换路径格式
print("Replacing keyword")
for entry in data:
    entry['file'] = entry['file'].replace('/mnt/c', 'C:/')
    entry['directory'] = entry['directory'].replace('/mnt/c', 'C:/')
    entry['command'] = entry['command'].replace('/mnt/c', 'C:/') 

with open('compile_commands.json', 'w') as file:
    json.dump(data, file, indent=4)

file.close()
