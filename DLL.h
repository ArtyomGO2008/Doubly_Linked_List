typedef struct
{
    int* data;
    int* next;
    int* prev;

    int capacity;
    int size;

    int head;
    int tail;
    int free;
} List;

#define ON_VERIFY
