int rump_shmem_write(void *buff, unsigned int size, unsigned int srcvm, unsigned int dstvm);
void * rump_shmem_read(void *buff, unsigned int srcvm, unsigned int dstvm);
int rump_shmem_dequeue_size(unsigned int srcvm, unsigned int dstvm);

void ipintr(void *arg);
