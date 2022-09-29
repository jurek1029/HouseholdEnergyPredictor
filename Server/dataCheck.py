import json

with open("monthly.data","r") as f:
	file = f.read()
	
data = json.loads(file)

print("tempNow len:",len(data["tempNow"]))
print("tempMonth len:",len(data["tempMonth"]))
print("loadNow len:",len(data["loadNow"]))
print("loadMonth len:",len(data["loadMonth"]))
print("predNow len:",len(data["predNow"]))