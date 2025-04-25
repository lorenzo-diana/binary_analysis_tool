#ifndef LINKED_LIST
#define LINKED_LIST

#include "inc/data_structure.h"

void list_init(struct List *list, enum ListType type);
void list_destroy(struct List *list);
struct Node * list_append(struct List *list);
unsigned int list_len(struct List *list);
struct Node * list_get(struct List *list, unsigned int index);

#endif /* LINKED_LIST */
