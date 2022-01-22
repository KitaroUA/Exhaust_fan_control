import glob, os
os.chdir("src")


for e in ['*.htm', '*.css']:
	for file in glob.glob(e):
		file_object = open(file,'a')
		with open(file, 'r') as original: data = original.read()
		file_string = file.replace('.','_')
		with open(file, 'w') as modified: modified.write('const char '+file_string+'[] PROGMEM = R"rawliteral(\n' + data + '\n)rawliteral";')
		file_object.close()
