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
	//const int allflags = O_ACCMODE | O_CREAT | O_EXCL | O_TRUNC | O_APPEND | O_NOCTTY;

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

	/*
	 * flags must include one of the following access modes:
	 * O_RDONLY, O_WRONLY, or O_RDWR.  These request opening the file
	 *  read-only,  write-only, or read/write, respectively.
	 */
	int openflag = flags & (O_RDONLY | O_WRONLY | O_RDWR);
	if (!( 	(openflag == O_RDONLY) ||
		(openflag == O_WRONLY) ||
		(openflag == O_WRONLY) ))
	{
		return EINVAL;
	}

	if ((flags & O_EXCL) && !(flags & O_CREAT)) {
		return EINVAL;
	}

	int len = strlen((const char*)upath)+1;
	kpath = (char*)kmalloc(len);
	if (kpath == NULL) {
		return ENOMEM;
	}
	
	result = copyin(upath,kpath,len);
	if (result) {
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

	struct filetable *ft = curproc->p_filetable;
	result = filetable_place(ft, file, retval);
	if (result) {
		// note refcound = 1 when struct openfile is created
		openfile_decref(file);
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
int
sys_write(int fd, userptr_t buf, size_t size, int *retval)
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
 * close() - remove from the file table.
 */
int
sys_close(int fd, int *retval)
{
	int result = 0;

	(void) fd;
	(void) retval;

	return result;
}

/* 
* encrypt() - read and encrypt the data of a file
*/
