import glob, os
os.chdir("src")


for wildcard in ['*.htm', '*.css']:
	for file in glob.glob(wildcard):
		file_object = open(file,'r+')
		file_string = file.replace('.','_')
		data = ''.join([line for line in file_object if not (line.startswith('const char '+file_string+'[] PROGMEM = R"rawliteral(') or line.startswith(')rawliteral";') )])
		data = data.rstrip('\n')                                                    
		file_object.seek(0)                                                         
		file_object.write(data) 
		file_object.truncate()    
		file_object.close()
