/*
 * encrypttest.c
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <err.h>

int cyc32r(int x, int n);

int
cyc32r(int x, int n)
{
	n = n % 32;
	if (!n) return x;
	return (x>>n) | (x<<(32-n));
}

#define BUFFSIZE 40
#define RDBUFFSIZE 41
#define WRBUFFSIZE 45

int
main(int argc, char *argv[])
{
	static char writebuf[WRBUFFSIZE] = "Twiddle dee dee, Twiddle dum dum.......\n";
	static char readbuf[RDBUFFSIZE];

	const char *file;
	int fd, rv;

	if (argc == 0) {
		warnx("No arguments - running on \"testfile\"");
		file = "testfile";
	}
	else if (argc == 2) {
		file = argv[1];
	}
	else {
		errx(1, "Usage: filetest <filename>");
	}

	printf("Got to write\n");

	// Write data and encrypt
	fd = open(file, O_RDWR|O_CREAT|O_TRUNC, 0664);
	if (fd<0) {
		err(1, "%s: open for write", file);
	}
	rv = write(fd, writebuf, 40);
	if (rv<0) {
		err(1, "%s: write", file);
	}
	printf("Got to encrypt\n");
	rv = encrypt(fd);
	printf("Finished encrypt\n");
	if (rv<0) {
		err(1, "%s: encrypt", file);
	}
	rv = close(fd);
	if (rv<0) {
		err(1, "%s: close (1st time)", file);
	}

	printf("Got to read\n");

	// read in data
	fd = open(file, O_RDONLY);
	if (fd<0) {
		err(1, "%s: open for read", file);
	}
	rv = read(fd, readbuf, 40);
	if (rv<0) {
		err(1, "%s: read", file);
	}
	readbuf[BUFFSIZE] = '\0';
	rv = close(fd);
	if (rv<0) {
		err(1, "%s: close (2nd time)", file);
	}

	printf("Got to user encrypt\n");

	// encrypt original data in userland (with 4 byte rule)
	int i=0;
	for (; i<BUFFSIZE; i+=4) {
		int temp = 0;
		if (i+4 < BUFFSIZE) {
			temp = (int)writebuf[i];
		}
		else {
			for (int j=0; j<BUFFSIZE%4; j++) {
				temp |= (writebuf[i+j] << 8*(4-j));
			}
			for (int j=BUFFSIZE%4; j<4; j++) {
				temp |= (' ' << 8*(4-j));
			}
		}
		temp = cyc32r(temp, 10);
		writebuf[i] = temp;
	}
	writebuf[i+1] = '\0';

	printf("Got to comparison\n");

	// compare results, should be identical
	if (strcmp(readbuf, writebuf)) {
		errx(1, "Buffer data mismatch!");
	}
	else {
		printf("Success!\n");
	}

	return 0;
}
