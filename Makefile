TARGETS = dl.so dl.o main-longjmp main-exceptions
all: $(TARGETS)
clean:
	rm -f $(TARGETS)
main-exceptions: main.c dl.c
	gcc -Wall -Werror -fexceptions -o main-exceptions main.c dl.c -ldl -lpthread
main-longjmp: main.c dl.c
	gcc -Wall -Werror              -o main-longjmp    main.c dl.c -ldl -lpthread
dl.so: dl.o
	ld -shared -o dl.so dl.o
dl.o: dl.c
	gcc -Wall -Werror -fexceptions -fPIC -c -o dl.o dl.c
