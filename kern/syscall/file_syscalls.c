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

/*
 * open() - get the path with copyinstr, then use openfile_open and
 * filetable_place to do the real work.
 */
int
sys_open(const_userptr_t upath, int flags, mode_t mode, int *retval)
{
	const int allflags = O_ACCMODE | O_CREAT | O_EXCL | O_TRUNC | O_APPEND | O_NOCTTY;

	char *kpath;
	struct openfile *file;
	int result = 0;

	/* 
	 * Your implementation of system call open starts here.  
	 *
	 * Check the design document design/filesyscall.txt for the steps
	 */
	(void) mode; // suppress compilation warning until code gets written
	(void) retval; // suppress compilation warning until code gets written
	(void) file; // suppress compilation warning until code gets written

	/*
	 * flags must include one of the following access modes:
	 * O_RDONLY, O_WRONLY, or O_RDWR.  These request opening the file
	 *  read-only,  write-only, or read/write, respectively.
	 */
	int openflag = flags & (O_RDONLY | O_WRONLY | O_RDWR);
	if (!( 	(openflag == O_RDONLY) ||
		(openflag == O_WRONLY) ||
		(openflag == O_WRONLY) 		))
	{
		errno = EINVAL;
		return -1;
	}

	int len = strlen(upath)+1;
	kpath = (char*)kmalloc(len);
	if (kapth == NULL) {
		errno = ENOMEM;
		return -1;
	}
	
	result = copyin(upath,kpath,len);
	if (result) {
		kfree(kpath);
		return result;
	}

	return result;
}

/*
 * read() - read data from a file
 */
int
sys_read(int fd, userptr_t buf, size_t size, int *retval)
{
       int result = 0;

       /* 
        * Your implementation of system call read starts here.  
        *
        * Check the design document design/filesyscall.txt for the steps
        */
       (void) fd; // suppress compilation warning until code gets written
       (void) buf; // suppress compilation warning until code gets written
       (void) size; // suppress compilation warning until code gets written
       (void) retval; // suppress compilation warning until code gets written

       return result;
}

/*
 * write() - write data to a file
 */

/*
 * close() - remove from the file table.
 */

/* 
* encrypt() - read and encrypt the data of a file
*/
