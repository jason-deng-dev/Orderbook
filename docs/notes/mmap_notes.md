# understanding mmap, the workhorse behind keeping memory access efficient in linux
https://www.youtube.com/watch?v=8hVLcyBkSXY

## Sharing pages
- virtual pages are mapped to a given process
- processes can have their own individual address spaces that map to whatever they want
- address translation (page table) translates teh virtual pages of the processes to physical memory
  - page table allow us to map any given virutal address to a physical address (in this direction only)
  - thus processes can share physical memory

## Demand Paging
- all pages start out as backed to the disk, and are marked as invalid when they are mapped into memory
  - such that the mapping exists between a specifc address of virtual memory to a specific offset within a file
  - but the actual bytes have not been pulled into memory yet
- means we never bring in things we never use
  - gives us Lazy loading property

Benefits:
- we don't expand resources loading anythign into memory that we aren't going to eventually use

# mmap/munmap (man7.org)
(https://man7.org/linux/man-pages/man2/mmap.2.html)
```c
#include <sys/mman.h>
void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
int munmap(void *addr, size_t length);
```
mmap() creates a new mapping in the virtual address space of the calling process
  - the starting address for the new mapping is specified in `addr`
  - `length` argument specifies the lenght of the mapping (must be greater than 0)
  - the contents of a file mapping are intialized using `length` bytes starting at `offset` in the file referred to by the file descriptor `fd
  - `offset` must be a multiple of the page size as returned by `sysconf(_SC_PAGE_SIZE)`
- `prot` describes the desired memory protection of the mapping (must not conflict with the open mode of the file)
  - PROT_EXEC: pages may be executed
  - PROT_READ: pages may be read
  - PROT_WRITE: pages may be written
  - PROT_NONE: pages may not be accesses
- `flags` determines whether updates to the mapping are visible to other processes mapping the same region, and whether updates are carried through to the underlying file

munmap() system call deletes the mappings for the specified address range
- cause further references to addresses within the range to generate invalid memory references
- the region is also automatically unmapped when the process is terminated
- closing file descriptor does not unmap the region
- address `addr` must be a multiple of page size (but `length` need not be)
- all pages containing a part of the indicated range are unmapped, and subseqeunt references to these pages will generte SIGSEGV

Return values:
on success, mmap() returns pointer to the mapped area
- on error the value MAP_FAILED is returned and `errno` is set to indicate the error

on succes, munmap() returns 0. 
- On failure returns -1, and `errno` is set to indicate error

# madvise (man7.org)
(https://man7.org/linux/man-pages/man2/madvise.2.html)
```c
#include <sys/mman.h>

int madvise(void *addr, size_t size, int advice)
```
- used to give advice or directions to kernel about hte address range beginning at address `addr` and `size
- only operates on whole pages, therefore addr must be page-aligned
- the value of size is rounded up to a multiple of page size
- goal of such advice is to improve system or application performance

# Usage
traditional way to read a file
```c
buffer_.resize(file_size);
while (total < file_size) {
    ssize_t n = read(fd, buffer_.data() + total, file_size - total);
    if (n == -1 && errno == EINTR) continue;
    total += n;
}
close(fd);
```
- heap alloc + full-file copy


mmap()
```c
void map = mmap(nullptr, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
close(fd);
madvise(map, file_size, MADV_SEQUENTIAL);
buffer_ = static_cast<const uint8_t*>(map);

// dtor: munmap(buffer_, file_size);
```
- no alloc, no copy (points at page cache)
- must munmap in dtor
- PROT_READ : read-only
- MAP_PRIVATE: private mapping, reads see the file, writes would Copy-on-Write
- close(fd): can close the file descriptor since the mapping holds its own reference
- MADV_SEQUENTIAL : tells kernel we read linearly, so it read-aheads agressively
- static_cast<const uint8_t*> since we want byte-level access for parsing
