#include <stdlib.h>
#include <stdint.h>
#include "linked_list.h"

// defined in loader.h
extern void symbol_init(struct Symbol *sym);
extern void symbol_destroy(struct Symbol *sym);
extern void section_init(struct Section *sec);
extern void section_destroy(struct Section *sec);

void list_init(struct List *list, enum ListType type)
{
    if (list)
    {
        list->head=list->tail=nullptr;
        list->list_type=type;
        list->len=0u;
    }
}

#define NODE_EL_DESTROY(el_pointer) _Generic((el_pointer), \
    struct Section *: section_destroy, \
    struct Symbol *: symbol_destroy \
)(el_pointer)

void list_destroy(struct List *list)
{
    if (!list)
        return;

    struct Node *t=list->head, *t_free;
    while (t)
    {
        if (list->list_type == SECTION_LIST)
            NODE_EL_DESTROY(&t->el.sec_el);
        else
            NODE_EL_DESTROY(&t->el.sym_el);
        t_free=t;
        t=t->next;
        free(t_free);
    }
}

#define NODE_EL_INIT(el_pointer) _Generic((el_pointer), \
    struct Section *: section_init, \
    struct Symbol *: symbol_init \
)(el_pointer)

struct Node * list_append(struct List *list)
{
    if (!list || list->len>=UINT32_MAX)
        return nullptr;

    struct Node * new_node = (struct Node *) malloc(sizeof(struct Node));
    if (!new_node)
        return nullptr;

    if (list->list_type==SECTION_LIST)
        NODE_EL_INIT(&new_node->el.sec_el);
    else
        NODE_EL_INIT(&new_node->el.sym_el);
    new_node->next=nullptr;

    if (list->tail) // if the list tail exists then new_node is appended to the end of the list, otherwise new_el is the head and tail of the list
    {
        list->tail->next = new_node;
        list->tail = new_node;
    }
    else
        list->head=list->tail = new_node;

    list->len += 1;

    return new_node;
}

unsigned int list_len(struct List *list)
{
    return list->len;
}

struct Node * list_get(struct List *list, unsigned int index)
{
    if (!list)
        return nullptr;
    struct Node *t=list->head;
    while (index-- && t)
        t=t->next;
    return t;
}
