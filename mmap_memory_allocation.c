#define MAP_ANON 0x20
#include <stdio.h>
#include <sys/mman.h>

void *malloc(size_t size)
{
    size_t len = size + sizeof(size_t);
    void *block = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    if (block == MAP_FAILED)
    {
        return NULL; 
    }
    *((size_t *)block) = len;
    return (void *)((char *)block + sizeof(size_t));
}

void free(void* ptr) {
    if (ptr == NULL) {
        return; 
    }

    void* block = (void*)((char*)ptr - sizeof(size_t));

    size_t len = *((size_t*)block);

    if (munmap(block, len) == -1) {
        perror("munmap failed");
    }
}

int main(int argc, char* argv[]){
    int* a = malloc(sizeof(int));
    *a = 17;
    printf("%d\n", *a);
    free(a);

}
