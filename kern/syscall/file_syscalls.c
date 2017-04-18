/*
 * File-related system call implementations.
 */

#include <types.h>
#include <kern/errno.h>
#include <kern/fcntl.h>
#include <kern/limits.h>
#include <kern/seek.h>
#include <kern/stat.h>
#include <lib.h>
#include <uio.h>
#include <proc.h>
#include <current.h>
#include <synch.h>
#include <copyinout.h>
#include <vfs.h>
#include <vnode.h>
#include <openfile.h>
#include <filetable.h>
#include <syscall.h>

int cyc32r(int x, int n);

/*
 * open() - get the path with copyinstr, then use openfile_open and
 * filetable_place to do the real work.
 */
int
sys_open(const_userptr_t upath, int flags, mode_t mode, int *retval)
{
	//const int allflags = O_ACCMODE | O_CREAT | O_EXCL | O_TRUNC | O_APPEND | O_NOCTTY;

	char *kpath;
	struct openfile *file;
	struct filetable *ft = curproc->p_filetable;
	int result = 0;
	int fd;

	/* 
	 * Check the design document design/filesyscall.txt for the steps
	 */
	(void) mode; // suppress compilation warning until code gets written

	/*
	 * flags must include one of the following access modes:
	 * O_RDONLY, O_WRONLY, or O_RDWR.  These request opening the file
	 *  read-only,  write-only, or read/write, respectively.
	 */
	int openflag = flags & (O_RDONLY | O_WRONLY | O_RDWR);
	if (!( 	(openflag == O_RDONLY) ||
		(openflag == O_WRONLY) ||
		(openflag == O_RDWR) ))
	{
		kprintf("sys_open given conflicting access mode flags\n");
		return EINVAL;
	}

	if ((flags & O_EXCL) && !(flags & O_CREAT)) {
		kprintf("sys_open given conflicting access mode flags\n");
		return EINVAL;
	}

	size_t len = strlen((const char*)upath)+1;
	kpath = (char*)kmalloc(len);
	if (kpath == NULL) {
		kprintf("sys_open memory error\n");
		return ENOMEM;
	}
	
	size_t actual;
	result = copyinstr(upath,kpath,len,&actual);
	//kprintf("%s\n", kpath);
	if (result || actual<len) {
		kfree(kpath);
		return result;
	}

	result = openfile_open(kpath, flags, mode, &file);
	if (result) {
		/*
		if (kpath!=null)
			kfree(kpath);
		*/
		return result;
	}

	result = filetable_place(ft, file, &fd);
	if (result) {
		// note refcound = 1 when struct openfile is created
		openfile_decref(file);
		kprintf("filetable_place issue\n");
		return result;
	}

	*retval = fd;
	return result;
}

/*
 * read() - read data from a file
 */
int
sys_read(int fd, userptr_t buf, size_t size, int *retval)
{
	int result = 0;
	struct openfile *file;
	struct filetable *ft = curproc->p_filetable;

	// uio variables
	struct vnode *rv;
	struct iovec iov;
	struct uio ku;
	off_t rpos;

	/* 
	 * Check the design document design/filesyscall.txt for the steps
	 */

	if (!filetable_okfd(ft,fd)) {
		kprintf("Bad filedescriptor %d \n", fd);
		return EBADF;
	}

	result = filetable_get(ft, fd, &file);
	if (result)
		return result;
	
	if (file->of_accmode & O_WRONLY) {
		kprintf("Bad filedescriptor %d \n", fd);
		return EBADF;
	}
	
	lock_acquire(file->of_offsetlock);

	// shamelessly lifted from kern/test/fstest.c
	rpos = file->of_offset;
	uio_kinit(&iov, &ku, buf, size, rpos, UIO_READ);

	rv = file->of_vnode;
	result = VOP_READ(rv, &ku);
	if (result) {
		lock_release(file->of_offsetlock);
		kprintf("VOP_READ error\n");
		return EIO;
	}
	file->of_offset = rpos + ku.uio_offset;

	lock_release(file->of_offsetlock);

	filetable_put(ft, fd, file);

	*retval = ku.uio_offset;
	return result;
}

/*
 * write() - write data to a file
 */
int
sys_write(int fd, userptr_t buf, size_t size, int *retval)
{
	int result = 0;
	struct openfile *file;
	struct filetable *ft = curproc->p_filetable;

	// uio variables
	struct vnode *rv;
	struct iovec iov;
	struct uio ku;
	off_t rpos;

	/* 
	 * Check the design document design/filesyscall.txt for the steps
	 */

	if (!filetable_okfd(ft,fd)) {
		kprintf("Bad filedescriptor %d \n", fd);
		return EBADF;
	}

	result = filetable_get(ft, fd, &file);
	if (result)
		return result;
	
	if (file->of_accmode & O_RDONLY) {
		kprintf("Bad filedescriptor %d \n", fd);
		return EBADF;
	}
	
	lock_acquire(file->of_offsetlock);

	// shamelessly lifted from kern/test/fstest.c
	rpos = file->of_offset;
	uio_kinit(&iov, &ku, buf, size, rpos, UIO_WRITE);

	rv = file->of_vnode;
	result = VOP_WRITE(rv, &ku);
	if (result) {
		lock_release(file->of_offsetlock);
		kprintf("VOP_WRITE error\n");
		return EIO;
	}
	file->of_offset = rpos + ku.uio_offset;

	lock_release(file->of_offsetlock);

	filetable_put(ft, fd, file);

	*retval = ku.uio_offset;
	return result;
}

/*
 * close() - remove from the file table.
 */
int
sys_close(int fd, int *retval)
{
	int result = 0;
	struct openfile *file, *old_file;
	struct filetable *ft = curproc->p_filetable;

	(void) retval;

	if (!filetable_okfd(ft,fd)) {
		kprintf("Bad filedescriptor %d \n", fd);
		return EBADF;
	}

	result = filetable_get(ft, fd, &file);
	if (result)
		return result;

	// doesn't fail
	filetable_placeat(ft, NULL, fd, &old_file);
	if (old_file == NULL)
		return EBADF;
	else
		openfile_decref(old_file);

	return result;
}

/* 
* encrypt() - read and encrypt the data of a file
*/

int
cyc32r(int x, int n)
{
	n = n % 32;
	if (!n) return x;
	return (x>>n) | (x<<(32-n));
}

int
sys_encrypt(int fd, int *retval)
{
	int result = 0;

	(void)retval;

	struct openfile *file;
	struct filetable *ft = curproc->p_filetable;

	// uio variables
	struct vnode *rv;
	struct iovec iov;
	struct uio ku;
	off_t  rpos, wpos;

	//int buf;
	bool done=false;

	if (!filetable_okfd(ft,fd)) {
		kprintf("Bad filedescriptor %d \n", fd);
		return EBADF;
	}

	result = filetable_get(ft, fd, &file);
	if (result)
		return result;
	
	if (!(file->of_accmode & O_RDWR)) {
		kprintf("Bad filedescriptor %d \n", fd);
		return EBADF;
	}
	
	// shamelessly lifted from kern/test/fstest.c
	rv = file->of_vnode;
	rpos = 0;
	wpos = 0;

	// Do we need to lock it?
	//lock_acquire(file->of_offsetlock);
	while(!done) {
		char buf[4];

		uio_kinit(&iov, &ku, &buf, sizeof(int), rpos, UIO_READ);
		result = VOP_READ(rv, &ku);
		if (result) {
			kprintf("VOP_READ error\n");
			return EIO;
		}
		rpos = ku.uio_offset;

		if (ku.uio_resid > 0) {
			/*  Seems to be ignored for some reason, so removed padding with ' ' in user-space version
			for (unsigned int i=0; i<ku.uio_resid; i++)
				buf[3-i] = ' ';
			*/
			done = true;
		}

		int bufnum = cyc32r((int)buf[3], 10);
		for (int j=0; j<4; j++)
			buf[j] = (char)((bufnum >> 8*j) & 0xff);

		// don't use uio_resid to align write, only allow 4 byte writes
		uio_kinit(&iov, &ku, &buf, sizeof(int), wpos, UIO_WRITE);
		result = VOP_WRITE(rv, &ku);
		if (result) {
			kprintf("VOP_WRITE error\n");
			return EIO;
		}
		wpos = ku.uio_offset;
	}
	//lock_release(file->of_offsetlock);

	filetable_put(ft, fd, file);

	return result;
}
