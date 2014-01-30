all:
	gcc -g sid-test.c rpi-sid.c -o sid-test -lpthread -ltca6416a -L . -D SID_DEBUG=0

clean:
	rm -f *.o sid-test
