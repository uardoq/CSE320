### This program is a server and is similar to Memcached.

```
./bin/cream     [-h] NUM_WORKERS PORT_NUMBER MAX_ENTRIES
-h				Displays this help menu and returns EXIT_SUCCESS.
NUM_WORKERS		The number of worker threads used to service requests.
PORT_NUMBER		Port number to listen on for incoming connections.
MAX_ENTRIES		The maximum number of entries that can be stored in `cream`'s underlying data store.
```

#### Notes:
* The server follows the "C.R.E.A.M" protocol, where the client sends a request header,
the server responds back with a response followed by a payload depending on the request.
* the server serves as an a multi-threaded in-memory storage by using a concurrent hashmap and a queue.  
* The queue uses semaphores to wait for access to the hashmap. When the queue is full, no more enqueueing happens until there is a spot frees up. See Producer-Consumer problem.
* The hashmap uses a mutex to lock access to modifying the hashmap data. 
* So while there can be multiple readers, there can only be 1 writer. See Reader-Writers problem.
* The server can handle requests like: Put, Get, Evict, Clear.