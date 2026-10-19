# Kernel Exploitation - Slab Allocators
https://www.youtube.com/watch?v=4UucTCiQLXE


![alt text](image.png)
Terms:
**cache**
- cache is created for a specific size
- many different caches that are used within the kernel's allocator
  - theres a cache for 256 byte objects / 512 byte objects ...
  - if there is some specific size that needs to be managed by the heap, a cache is created specific for that size
- caches manage something called slabs (caches hold slabs)

**slabs**
- a slab is a contiguous region of memory
- can be one or more pages in size
- slabs consists of a series of slots

**slots**
- slot is a regiuon of memory within the slab, that is the size being managed by the cache
- slots can contain objects
- when a malloc occurs, the pointer that is returned is the pointer to one of these available slots inside the slab
- once that available slot is being used, "slot contains an object"

Benefits of slab allocator approach
- once objects are initialzied, that initalzied data can be reused when possible
- normally, when we call malloc => free => malloc 
  - we have no guarantees about the values of the memory that is returned to us
- with slab implementation
  - if cache is created for a specific data structure, and there are kind of static values
  - then once all of the slots have been initalized with that static value
  - we can assume that the data is already there when we obtain the allocation
  - because the only things that these region of memory are being used for are those specific data structures
