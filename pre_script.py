import glob, os
#os.chdir("src")
for e in ['./web/*.htm', './web/*.css', './web/*.js', './web/*.bmp']:
	for file in glob.glob(e):
		print(file[6:])
		with open(file, 'r') as original: lines = original.readlines()
		file_string = file[6:].replace('.','_')
		file_string = file_string.replace('-','_')
		with open(file, 'r') as original: data = original.read()
		if not ( lines[0].startswith('const char '+file_string+'[] PROGMEM = R"rawliteral(') ) :
			print("add start")
			data = 'const char '+file_string+'[] PROGMEM = R"rawliteral(\n' + data
		if not ( lines[-1].startswith(')rawliteral";') ) :
			print("add end")
			data = data + '\n)rawliteral";'
		with open('./src/'+file[6:]+'.h', 'w') as modified: modified.write(data)
#os.chdir("..")
