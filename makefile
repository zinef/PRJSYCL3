compiler:
	gcc -g  main.c -o zfi -lreadline

exec:
	./zfi

image:
	sudo docker build -t zfishell .
	
debug:
	gdb ./zfi
	
run:
	sudo docker run -ti zfishell 
