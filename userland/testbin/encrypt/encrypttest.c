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

#define BUFFSIZE 50

int
main(int argc, char *argv[])
{
	static char writebuf[BUFFSIZE] = " Twiddle dee dee, Twiddle dum dum.......1 2 3 4 ";
	static char readbuf[BUFFSIZE]  = "                                                ";

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

	// Write data and encrypt
	fd = open(file, O_RDWR|O_CREAT|O_TRUNC, 0664);
	if (fd<0) {
		err(1, "%s: open for write", file);
	}
	rv = write(fd, writebuf, BUFFSIZE);
	if (rv<0) {
		err(1, "%s: write", file);
	}
	rv = encrypt(fd);
	printf("Finished encrypt\n");
	if (rv<0) {
		err(1, "%s: encrypt", file);
	}
	rv = close(fd);
	if (rv<0) {
		err(1, "%s: close (1st time)", file);
	}

	// read in data
	fd = open(file, O_RDONLY);
	if (fd<0) {
		err(1, "%s: open for read", file);
	}
	rv = read(fd, readbuf, BUFFSIZE);
	if (rv<0) {
		err(1, "%s: read", file);
	}
	rv = close(fd);
	if (rv<0) {
		err(1, "%s: close (2nd time)", file);
	}

	// encrypt original data in userland (with 4 byte rule) [ but not padding with spaces ]
	int wrlen = strlen(writebuf);
	int maxwr = wrlen + wrlen % 4; // maxwr is the nearest multiple of 4 past wrlen
	for (int i=0; i<maxwr; i+=4) {
		char buf[4];
		for (int j=0; j<4; j++)
			//if (i+j < wrlen) // for some reason kernel-space version doesn't pad with ' ' but still works without crashing
				buf[j] = writebuf[i+j];
		int bufnum = cyc32r((int)buf[3], 10);
		for (int j=0; j<4; j++)
			writebuf[i+j] = (char)((bufnum >> 8*j) & 0xff);
	}

	// compare results, should be identical
	printf("\n File:"); for (unsigned int i=0; i<BUFFSIZE; i++) printf("%d ", readbuf[i]);
	printf("\n Buff:"); for (unsigned int i=0; i<BUFFSIZE; i++) printf("%d ", writebuf[i]);
	if (memcmp(readbuf, writebuf, BUFFSIZE)) {
		printf("\nBuffer data mismatch!\n");
		return 1;
	}
	else {
		printf("\nSuccess!\n");
		return 0;
	}
}
