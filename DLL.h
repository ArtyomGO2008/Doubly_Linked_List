#include <stdio.h>

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

enum ListErrors
{
    LIST_OK              = 0,
    LIST_NULL_PTR        = -1,
    LIST_BAD_CAPACITY    = -2,
    LIST_INVALID_INDEX   = -3,
    LIST_INCORRECT_VALUE = -4,
    LIST_BROKEN_LINKS    = -5,
    LIST_CYCLE_DETECTED  = -6,
    LIST_MISSED_ELEM     = -7,
    LIST_OVERFLOW        = -8,
};


#define ON_VERIFY
#define LIST_DUMP(list,...) do { \
    char buffer[256] = ""; \
    snprintf(buffer, sizeof(buffer), __VA_ARGS__); \
    ListDump (list, buffer); \
} while (0)

void CreateLogDir (void);
void PrintSentinel (FILE* file, const List* list, int i);
void PrintHead     (FILE* file, const List* list, int i);
void PrintTail     (FILE* file, const List* list, int i);
void PrintFree     (FILE* file, const List* list, int i);
void PrintUsed     (FILE* file, const List* list, int i);
void ListDump (const List* list, const char* message);
void WriteTextDump (const List* list, FILE* html_file);
void WriteHTML (const List* list, FILE* html_file, int dump_num, const char* message);
void WriteDot (const List* list, FILE* dot_file);

