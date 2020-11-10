
ping-report : src/ping-report.c
	gcc -o ping-report src/ping-report.c

clean :
	rm -f ping-report
