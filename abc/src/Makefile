main: FORCE
	gcc -Wall -g -c main.cpp -o main.o
	g++ -g -o main main.o libabc.a -lm -ldl -lreadline -lpthread
clean:
	rm main.o
	rm main
FORCE: ;
