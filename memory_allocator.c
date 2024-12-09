#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

typedef char ALIGN[16];

typedef union header header_t;
union header
{

    struct
    {
        size_t size;
        unsigned is_free;
        header_t *next;
    } s;

    ALIGN stub;
};

header_t *head, *tail;

pthread_mutex_t global_malloc_lock;

header_t *get_free_block(size_t size)
{
    header_t *curr = head;
    while (curr != NULL)
    {
        if (curr->s.size <= size && curr->s.is_free == 1)
        {
            return curr;
        }
        curr = curr->s.next;
    }
    return NULL;
}

void *malloc(size_t size)
{
    void *block;
    header_t *header;
    if (!size)
    {
        return NULL;
    }
    pthread_mutex_lock(&global_malloc_lock);
    header = get_free_block(size);
    if (header)
    {
        header->s.is_free = 0;
        pthread_mutex_unlock(&global_malloc_lock);
        return (void *)(header + 1);
    }
    size_t total_size = sizeof(header_t) + size;
    block = sbrk(total_size);
    if (block == (void *)-1)
    {
        pthread_mutex_unlock(&global_malloc_lock);
        return NULL;
    }
    header = block;
    header->s.is_free = 0;
    header->s.size = size;
    header->s.next = NULL;
    if (!head)
    {
        head = header;
    }
    if (tail)
    {
        tail->s.next = header;
    }
    tail = header;
    pthread_mutex_unlock(&global_malloc_lock);
    return (void *)(header + 1);
}

void free(void *block)
{
    if (!block)
    {
        return;
    }
    pthread_mutex_lock(&global_malloc_lock);
    header_t *header = (header_t *)block - 1;
    header_t *tmp;
    void *programbreak = sbrk(0);
    if ((char *)block + header->s.size == programbreak)
    {
        if (head == tail)
        {
            head = tail = NULL;
        }
        else
        {
            tmp = head;
            while (tmp)
            {
                if (tmp->s.next == tail)
                {
                    tmp->s.next = NULL;
                    tail = tmp;
                }
                tmp = tmp->s.next;
            }
        }
        sbrk(0 - sizeof(header_t) - header->s.size);
        pthread_mutex_unlock(&global_malloc_lock);
        return;
    }

    header->s.is_free = 1;
}

int main(int argc, char *argv[])
{
    int *hello = malloc(sizeof(int));
    if (!hello)
    {
        printf("Unable to allocate memory\n");
    }
    else
    {
        printf("Memory allocated\n");
        *hello = 17;
        printf("Stored number is %d\n", *hello);

        free((void *)hello);
        hello = NULL; // Nullify pointer after freeing
    }

    return 0;
}
