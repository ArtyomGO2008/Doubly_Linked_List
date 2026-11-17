#include <stdio.h>
#include <cstdlib>
#include <direct.h>
#include <string.h>
#include <errno.h>
#include "DLL.h"

#define POISON 0xDEAD
#define LOG_DIR "List_logs"
#define MKDIR(path) _mkdir(path)

void CreateLogDir (void)
{
    static int is_created = 0;
    if (is_created) return;

    if (MKDIR(LOG_DIR) == 0 || errno == EEXIST)
        is_created = 1;
    else
        fprintf(stderr, "Error creating the folder: %s\n", strerror(errno));
}

void WriteDot (const List* list, FILE* dot_file)
{
    fprintf(dot_file, "digraph G\n");
    fprintf(dot_file, "{\n");
    fprintf(dot_file, "\trankdir = LR\n");
    fprintf(dot_file, "\tsplines = ortho\n");

    fprintf(dot_file, "\tnode [shape = Mrecord, style = filled]\n");

    for (int i = 0; i <= list->capacity; i++)
    {
        if (i == 0)
            PrintSentinel(dot_file, list, i);
        else if (i == list->head)
            PrintHead(dot_file, list, i);
        else if (i == list->tail)
            PrintTail(dot_file, list, i);
        else if (list->prev[i] == -1 && list->data[i] == POISON)
            PrintFree(dot_file, list, i);
        else
            PrintUsed(dot_file, list, i);
    }

    fprintf(dot_file, "\n\tV0 ");
    for (int i = 1; i <= list->capacity; i++)
        fprintf(dot_file, "-> V%d ", i);
    fprintf(dot_file, "[style = invis; weight = 1000]\n\n");

    for (int i = 0; i <= list->capacity; i++)
    {
        if (list->prev[i] == -1 && list->data[i] == POISON)
            continue;
        for (int j = 0; j <= list->capacity; j++)
        {
            if (list->prev[j] == -1 && list->data[j] == POISON)
                continue;
            if (list->next[i] == j && list->prev[j] == i)
                fprintf(dot_file, "\tV%d -> V%d [dir = both]\n", i, j);
            else if (list->next[i] == j)
                fprintf(dot_file, "\tV%d -> V%d [dir = forward; color = \"#FF0000\"]\n", i, j);
            else if (list->prev[j] == i)
                fprintf(dot_file, "\tV%d -> V%d [dir = forward; color = \"#FF0000\"]\n", j, i);
        }
    }

    fprintf(dot_file, "}\n");
}

void WriteHTML (const List* list, FILE* html_file, int dump_num, const char* message)
{
    if (dump_num == 1)
    {
        fprintf(html_file, "<pre>\n");
    }

    fprintf(html_file, "<h3>DUMP <font color=\"red\">%s</font></h3>", message);
    fprintf(html_file, "List { DLL.cpp }\n\n");
    TEXT_ListDump (list, html_file);
    fprintf(html_file, "\nImage:\n");
    fprintf(html_file, "<img src=list_graph_%03d.png width=1000px>\n\n", dump_num);
}

void PrintSentinel (FILE* file, const List* list, int i)
{
    fprintf(file, "\tV%d [fillcolor = \"#d94797\"; color = \"#000000\"; label = \"INDEX = %d|DATA = POISON|{NEXT = %d|PREV = %d}\"]\n",
            i, i, list->next[i], list->prev[i]);
}

void PrintHead (FILE* file, const List* list, int i)
{
    fprintf(file, "\tV%d [fillcolor = \"#f4a629\"; color = \"#000000\"; label = \"INDEX = %d|DATA = %d|{NEXT = %d|PREV = %d}\"]\n",
            i, i, list->data[i], list->next[i], list->prev[i]);
    fprintf(file, "\tHEAD [fillcolor = \"#f4a629\"; color = \"#000000\"; label = \"HEAD\"]\n");
    fprintf(file, "\tHEAD -> V%d [color = \"#f4a629\"; style = \"bold\"]\n", i);
}

void PrintTail (FILE* file, const List* list, int i)
{
    fprintf(file, "\tV%d [fillcolor = \"#ad29f4\"; color = \"#000000\"; label = \"INDEX = %d|DATA = %d|{NEXT = %d|PREV = %d}\"]\n",
            i, i, list->data[i], list->next[i], list->prev[i]);
    fprintf(file, "\tTAIL [fillcolor = \"#ad29f4\"; color = \"#000000\"; label = \"TAIL\"]\n");
    fprintf(file, "\tTAIL -> V%d [color = \"#ad29f4\"; style = \"bold\"]\n", i);
}

void PrintFree (FILE* file, const List* list, int i)
{
    fprintf(file, "\tV%d [fillcolor = \"#C0FFC0\"; color = \"#000000\"; label = \"INDEX = %d|DATA = POISON|{NEXT = %d|PREV = %d}\"]\n",
            i, i, list->next[i], list->prev[i]);
    if (list->free == i)
    {
        fprintf(file, "\tFREE [fillcolor = \"#C0FFC0\"; color = \"#000000\"; label = \"FREE\"]\n");
        fprintf(file, "\tFREE -> V%d [color = \"#C0FFC0\"; style = \"bold\"]\n", i);
    }
}

void PrintUsed (FILE* file, const List* list, int i)
{
    fprintf(file, "\tV%d [fillcolor = \"#79b5f5\"; color = \"#000000\"; label = \"INDEX = %d|DATA = %d|{NEXT = %d|PREV = %d}\"]\n",
            i, i, list->data[i], list->next[i], list->prev[i]);
}

void WriteTextDump (const List* list, FILE* html_file)
{
    fprintf(html_file, "head = %d\n", list->head);
    fprintf(html_file, "tail = %d\n", list->tail);
    fprintf(html_file, "free = %d\n", list->free);
    fprintf(html_file, "size = %d\n\n", list->size);

    fprintf(html_file, "index |  data  | next | prev\n");
    fprintf(html_file, "------+--------+------+------\n");

    for (int i = 0; i <= list->capacity; i++)
    {
        if (list->data[i] == (int)POISON)
            fprintf(html_file, "%5d | 0x%X | %4d | %4d\n",
                    i, list->data[i], list->next[i], list->prev[i]);
        else
            fprintf(html_file, "%5d | %6d | %4d | %4d\n",
                    i, list->data[i], list->next[i], list->prev[i]);
    }
}

void ListDump (const List* list, const char* message)
{
    CreateLogDir();

    static int dump_num = 0;
    dump_num++;

    char dot_path[256] = "";
    char png_path[256] = "";
    char cmd[512] = "";

    snprintf(dot_path, sizeof(dot_path), "%s/list_graph_%03d.dot", LOG_DIR, dump_num);
    snprintf(png_path, sizeof(png_path), "%s/list_graph_%03d.png", LOG_DIR, dump_num);


    FILE* dot_file = fopen(dot_path, "w");
    if (dot_file == NULL)
    {
        fprintf(stderr, "Could not open .dot file");
        return;
    }

    WriteDot(list, dot_file);

    fclose(dot_file);

    snprintf(cmd, sizeof(cmd), "dot -Tpng %s -o %s", dot_path, png_path);
    int result = system(cmd);

    if (result != 0)
    {
        fprintf(stderr, "The command could not be executed.\n");
        return;
    }


    char html_path[256] = "";
    snprintf(html_path, sizeof(html_path), "%s/List_log.htm", LOG_DIR);

    FILE* html_file = fopen(html_path, (dump_num == 1) ? "w" : "a");

    if (html_file == NULL)
    {
        fprintf(stderr, "Error opening HTML file '%s': %s\n", html_path, strerror(errno));
        return;
    }

    WriteHTML (list, html_file, dump_num, message);

    fclose(html_file);
}

