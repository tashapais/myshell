mysh: mysh.c 
	gcc -c mysh.c -lreadline -o mysh
clean:
	rm *.o mysh mysh.exe