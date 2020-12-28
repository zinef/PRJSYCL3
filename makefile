compiler:
	gcc -g  main.c -o prgm -lreadline

exec:
	./prgm

image:
	sudo docker build -t zfishell .
	
debug:
	gdb ./prgm
	
run:
	sudo docker run -ti zfishell 
