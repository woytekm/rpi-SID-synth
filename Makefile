all:
	gcc sid-test.c rpi-sid.c -o sid-test -ltca6416a -L .

clean:
	rm -f *.o sid-test
