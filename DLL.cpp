#include "DLL.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


#define POISON 0xDEAD

int ListVerify (const List* list)
{
    if (!list || !list->data || !list->next || !list->prev) return LIST_NULL_PTR;
    if (list->capacity <= 0 || 0 > list->size || list->size > list->capacity) return LIST_BAD_CAPACITY;
    if (list->next[list->tail] != 0 || list->prev[list->head] != 0) return LIST_BROKEN_LINKS;

    if (list->head < 0 || list->head > list->capacity || list->tail < 0 ||
        list->tail > list->capacity || list->free < 0 || list->free > list->capacity) return LIST_INVALID_INDEX;

    if (list->size == 0)
    {
        if (list->head != 0 || list->tail != 0) return LIST_INCORRECT_VALUE;
    }
    else
    {
        if (list->head == 0 || list->tail == 0) return LIST_INCORRECT_VALUE;
    }

    for (int i = 0; i <= list->capacity; i++)
    {
        if (list->next[i] < 0  || list->capacity < list->next[i]) return LIST_INVALID_INDEX;
        if (list->prev[i] < -1 || list->capacity < list->prev[i]) return LIST_INVALID_INDEX;

        if (list->data[i] != POISON )
        {
            if (list->prev[list->next[i]] != i) return LIST_BROKEN_LINKS;
            if (list->next[list->prev[i]] != i) return LIST_BROKEN_LINKS;
        }

        else if (list->prev[i] == -1 && list->next[i] != 0)
            if (list->prev[list->next[i]] != -1) return LIST_BROKEN_LINKS;
    }

    if (list->data[0] != POISON ) return LIST_INCORRECT_VALUE;

    for (int i = 1; i <= list->capacity; i++)
    {
        if (list->prev[i] == -1)
        {
            if (list->data[i] != POISON) return LIST_INCORRECT_VALUE;
        }
        else
        {
            if (list->data[i] == POISON) return LIST_INCORRECT_VALUE;
        }
    }

    int count_free = 0;
    int count_used = 0;
    for (int i = 1; i <= list->capacity; i++)
    {
        if (list->prev[i] == -1)
            count_free += 1;
        else
            count_used +=1;
    }

    if (list->size != count_used) return LIST_MISSED_ELEM;

    if (list->size + count_free != list->capacity) return LIST_MISSED_ELEM;

//=============== проверка на циклы и потерю узлов ===============
    int* seen = (int*)calloc(list->capacity + 1, sizeof(int));

    if (!seen)
        return LIST_NULL_PTR;

    int next_elem = 0;
    int prev_elem = 0;

// считаем использованные элементы и проверяем на циклы основной список

    count_used = 0;

    for (int i = list->head; i != 0;)
    {
        if (i < 1 || i > list->capacity) return LIST_INVALID_INDEX;
        if (seen[i] != 0 || count_used > list->capacity) return LIST_CYCLE_DETECTED;

        seen[i] = 1;

        next_elem = list->next[i];

        if (next_elem < 0 || next_elem > list->capacity ||
            (next_elem != 0 && list->prev[next_elem] != i))
            return LIST_INVALID_INDEX;

        prev_elem = i;
        i = next_elem;

        count_used++;
    }

    if (count_used != list->size) return LIST_MISSED_ELEM;
    if (prev_elem != list->tail) return LIST_INVALID_INDEX;

// считаем свободные элементы и проверяем на циклы список свободных элементов

    count_free = 0;

    for (int i = list->free; i != 0;)
    {
        if (i < 1 || i > list->capacity) return LIST_INVALID_INDEX;
        if (seen[i] != 0 || list->prev[i] != -1) return LIST_CYCLE_DETECTED;

        seen[i] = 2;

        next_elem = list->next[i];

        if (next_elem < 0 || next_elem > list->capacity)
            return LIST_INVALID_INDEX;

        count_free++;
        i = next_elem;
    }

    if (count_used + count_free != list->capacity)
        return LIST_MISSED_ELEM;

    free(seen);

    return LIST_OK;
}

int ListInit (List* list, int capacity)
{
    if (list == NULL || capacity <= 0)
        return -1;

    list->data = (int*)calloc(capacity + 1, sizeof(int));
    list->next = (int*)calloc(capacity + 1, sizeof(int));
    list->prev = (int*)calloc(capacity + 1, sizeof(int));

    if (list->data == NULL || list->next == NULL || list->prev == NULL)
    {
        free(list->data);
        free(list->next);
        free(list->prev);

        return -1;
    }

    list->size = 0;
    list->capacity = capacity;

    list->head = 0;
    list->tail = 0;
    list->free = 1;

    list->data[0] = POISON;
    list->next[0] = 0;
    list->prev[0] = 0;

    for (int i = 1; i <= capacity; i++)
    {
        list->prev[i] = -1;
        list->next[i] = (i < capacity) ? i + 1 : 0;
        list->data[i] = POISON;
    }

    LIST_DUMP(list, "AFTER INIT");

    #ifdef ON_VERIFY
        int err = ListVerify(list);
        if (err < 0) return err;
    #endif

    return 0;
}

void ListDump (const List* list, const char* log_file_name)
{
    FILE* log_file = fopen(log_file_name, "w");

    fprintf(log_file, "head = %d\n", list->head);
    fprintf(log_file, "tail = %d\n", list->tail);
    fprintf(log_file, "free = %d\n", list->free);
    fprintf(log_file, "size = %d\n", list->size);

    fprintf(log_file, "index |  data  | next | prev\n");
    fprintf(log_file, "------+--------+------+------\n");

    for (int i = 0; i <= list->capacity; i++)
    {
        if (list->data[i] == (int)POISON)
            fprintf(log_file, "%5d | 0x%X | %4d | %4d\n",
                    i, list->data[i], list->next[i], list->prev[i]);
        else
            fprintf(log_file, "%5d | %6d | %4d | %4d\n",
                    i, list->data[i], list->next[i], list->prev[i]);
    }

    fclose (log_file);
}

int ListInsertAfter (List* list, int position, int value)
{
    LIST_DUMP(list, "BEFORE INSERT (%d, %d)", position, value);

    #ifdef ON_VERIFY
        int err = ListVerify(list);
        if (err < 0) return err;
    #endif

    if (list->free == 0)
        return LIST_OVERFLOW;

    int new_elem = list->free;
    list->free = list->next[list->free];

    list->data[new_elem] = value;

// без фиктивного элемента
//     if (list->size == 0)
//     {
//         list->head = new_elem;
//         list->tail = new_elem;
//         list->next[new_elem] = 0;
//         list->prev[new_elem] = 0;
//
//         list->size++;
//         return new_elem;
//     }
//
//     int next_elem = (position == 0) ? list->head : list->next[position];
//
//     list->next[new_elem] = next_elem;
//     list->prev[new_elem] = position;
//
//     if (position == 0)
//         list->head = new_elem;
//     else
//         list->next[position] = new_elem;
//
//     if (next_elem == 0)
//         list->tail = new_elem;
//     else
//         list->prev[next_elem] = new_elem;


// с фиктивным элементом
// находим следующий элемент, если position == 0 (вставка до головы) ,то new_elem = next[0] = head
// если вставляем в пустой список, то next_elem = 0;
    int next_elem = list->next[position];

// устанавливаем значение и связи для new_elem
    list->data[new_elem] = value;
    list->next[new_elem] = next_elem;
    list->prev[new_elem] = position;

// меняем связи соседей
    list->next[position] = new_elem;
    list->prev[next_elem] = new_elem;

// обновляем head и tail
    list->head = list->next[0];
    list->tail = list->prev[0];

    list->size++;

    LIST_DUMP(list, "AFTER INSERT (%d, %d)", position, value);

    #ifdef ON_VERIFY
        err = ListVerify(list);
        if (err < 0) return err;
    #endif

    return new_elem;
}

int ListDelete (List* list, int index)
{
    LIST_DUMP(list, "BEFORE DELETE (%d)", index);

    #ifdef ON_VERIFY
        int err = ListVerify(list);
        if (err < 0) return err;
    #endif

    if (index < 1 || list->capacity < index || list->data[index] == POISON)
        return LIST_INVALID_INDEX;

    int left = list->prev[index];
    int right = list->next[index];


// без фиктивного элемента
//     if (left == 0)
//         list->head = right;
//     else
//         list->next[left] = right;
//
//     if (right == 0)
//         list->tail = left;
//     else
//         list->prev[right] = left;

// с фиктивным элементом
// меняем связи соседей
    list->next[left] = right;
    list->prev[right] = left;

// прикрепляем удаленный элемент к списку свободных
    list->next[index] = list->free;
    list->prev[index] = -1;
    list->data[index] = POISON;
    list->free = index;

// меняем head и tail
    list->head = list->next[0];
    list->tail = list->prev[0];

    list->size--;

    LIST_DUMP(list, "AFTER DELETE (%d)", index);

    #ifdef ON_VERIFY
        err = ListVerify(list);
        if (err < 0) return err;
    #endif

    if (right)
        return right;
    if (left)
        return left;

    return 0;
}

int main()
{
    List list1 ={};
    ListInit(&list1, 10);
    ListInsertAfter (&list1, 0, 10);
    ListInsertAfter(&list1, 1, 20);
    ListInsertAfter(&list1, 2, 30);
    ListInsertAfter(&list1, 3, 40);
    ListInsertAfter(&list1, 4, 50);
    ListInsertAfter(&list1, 3, 35);
    ListDelete(&list1, 3);
    ListDelete(&list1, 5);
    ListInsertAfter(&list1, 4, 60);;
    ListInsertAfter(&list1, 6, 70);
    ListInsertAfter(&list1, 1, 80);
    ListInsertAfter(&list1, 7, 90);
    ListDelete(&list1, 1);
    ListDelete(&list1, 6);
    //printf("err = %d\n", err);
    return 0;
}
