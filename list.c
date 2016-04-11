#include "list.h"
#include <stdlib.h>

void list_init(struct list *l) {
    l->length = 0;
    l->head.prev = NULL;
    l->head.next = &l->tail;
    l->tail.prev = &l->head;
    l->tail.next = NULL;
}

/* adding to a list should always succeed, so no need to return any status */
void list_add(struct list *l, void *data) {
    l->length++;
    struct _list_node *n = malloc(sizeof(struct _list_node));
    n->data = data;
    /* append to end */
    struct _list_node *last = l->tail.prev;
    last->next = n;
    n->prev = last;
    n->next = &l->tail;
    l->tail.prev = n;
}

void *get_first(struct list *l, const void *prop, int (*recognize)(const void *, void *)) {
    struct _list_node *n;
    for (n = l->head.next; n != &l->tail; n = n->next) {
        if (recognize(prop, n->data)) {
            return n->data;
        }
    }
    return NULL;
}

/* returns the removed element */
void *list_remove_first(struct list *l, const void *prop, int (*recognize)(const void *, void *)) {
    if (l->length == 0) {
        return NULL;
    }
    l->length--;
    struct _list_node *n;
    for (n = l->head.next; n != &l->tail; n = n->next) {
        if (recognize(prop, n->data)) {
            break;
        }
    }
    if (n == &l->tail) {
        return NULL;
    }
    n->prev->next = n->next;
    n->next->prev = n->prev;
    void *data = n->data;
    free(n);
    return data;
}

/* returns number of elements removed */
int list_remove_all(struct list *l, const void *prop, int (*recognize)(const void *, void *)) {
    if (l->length == 0) {
        return 0;
    }
    int num_removed = 0;
    struct _list_node *n, *next;
    for (n = l->head.next; n != &l->tail; n = next) {
        next = n->next;
        if (recognize(prop, n->data)) {
            n->prev->next = next;
            next->prev = n->prev;
            free(n);
            num_removed++;
        }
    }
    l->length -= num_removed;
    return num_removed;
}

int list_contains(struct list *l, const void *prop, int (*recognize)(const void *, void *)) {
    struct _list_node *n;
    for (n = l->head.next; n != &l->tail; n = n->next) {
        if (recognize(prop, n->data)) {
            return 1;
        }
    }
    return 0;
}

void list_for_each(struct list *l, void (*f)(void *)) {
    struct _list_node *n;
    for (n = l->head.next; n != &l->tail; n = n->next) {
        f(n->data);
    }
}


