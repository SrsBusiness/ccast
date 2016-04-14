#include "list.h"
#include <stdlib.h>

void list_init(struct list *l) {
    pthread_mutex_init(&l->mutex, NULL);
    pthread_mutex_lock(&l->mutex);
    l->length = 0;
    l->head.prev = NULL;
    l->head.next = &l->tail;
    l->tail.prev = &l->head;
    l->tail.next = NULL;
    pthread_mutex_unlock(&l->mutex);
}

/* adding to a list should always succeed, so no need to return any status */
void _list_add(struct list *l, void *data, int sync) {
    if (sync) {
        pthread_mutex_lock(&l->mutex);
    }
    l->length++;
    struct _list_node *n = malloc(sizeof(struct _list_node));
    n->data = data;
    /* append to end */
    struct _list_node *last = l->tail.prev;
    last->next = n;
    n->prev = last;
    n->next = &l->tail;
    l->tail.prev = n;
    if (sync) {
        pthread_mutex_unlock(&l->mutex);
    }
}

void *_get_first(struct list *l, const void *prop, int (*recognize)(const void *, void *), int sync) {
    if (sync) {
        pthread_mutex_lock(&l->mutex);
    }
    struct _list_node *n;
    for (n = l->head.next; n != &l->tail; n = n->next) {
        if (recognize(prop, n->data)) {
            return n->data;
        }
    }
    if (sync) {
        pthread_mutex_unlock(&l->mutex);
    }
    return NULL;
}

/* returns the removed element */
void *_list_remove_first(struct list *l, const void *prop, int (*recognize)(const void *, void *), int sync) {
    if (sync) {
        pthread_mutex_lock(&l->mutex);
    }
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
    if (sync) {
        pthread_mutex_unlock(&l->mutex);
    }
    return data;
}

/* returns number of elements removed */
int _list_remove_all(struct list *l, const void *prop, int (*recognize)(const void *, void *), int sync) {
    if (sync) {
        pthread_mutex_lock(&l->mutex);
    }
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
    if (sync) {
        pthread_mutex_unlock(&l->mutex);
    }
    return num_removed;
}

int _list_contains(struct list *l, const void *prop, int (*recognize)(const void *, void *), int sync) {
    if (sync) {
        pthread_mutex_lock(&l->mutex);
    }
    struct _list_node *n;
    for (n = l->head.next; n != &l->tail; n = n->next) {
        if (recognize(prop, n->data)) {
            return 1;
        }
    }
    if (sync) {
        pthread_mutex_unlock(&l->mutex);
    }
    return 0;
}

void _list_for_each(struct list *l, void (*f)(void *), int sync) {
    if (sync) {
        pthread_mutex_lock(&l->mutex);
    }
    struct _list_node *n;
    for (n = l->head.next; n != &l->tail; n = n->next) {
        f(n->data);
    }
    if (sync) {
        pthread_mutex_unlock(&l->mutex);
    }
}

void list_add(struct list *l, void *data) {
    _list_add(l, data, 0);
}

void list_add_sync(struct list *l, void *data) {
    _list_add(l, data, 1);
}

void *get_first(struct list *l, const void *prop, int (*recognize)(const void *, void *)) {
    return _get_first(l, prop, recognize, 0);
}

void *get_first_sync(struct list *l, const void *prop, int (*recognize)(const void *, void *)) {
    return _get_first(l, prop, recognize, 1);
}

void *list_remove_first(struct list *l, const void *prop, int (*recognize)(const void *, void *)) {
    return _list_remove_first(l, prop, recognize, 0);
}

void *list_remove_first_sync(struct list *l, const void *prop, int (*recognize)(const void *, void *)) {
    return _list_remove_first(l, prop, recognize, 1);
}

int list_remove_all(struct list *l, const void *prop, int (*recognize)(const void *, void *)) {
    return _list_remove_all(l, prop, recognize, 0);
}

int list_remove_all_sync(struct list *l, const void *prop, int (*recognize)(const void *, void *)) {
    return _list_remove_all(l, prop, recognize, 1);
}

int list_contains(struct list *l, const void *prop, int (*recognize)(const void *, void *)) {
    return _list_contains(l, prop, recognize, 0);
}

int list_contains_sync(struct list *l, const void *prop, int (*recognize)(const void *, void *)) {
    return _list_contains(l, prop, recognize, 1);
}

void list_for_each(struct list *l, void (*f)(void *)) {
    return _list_for_each(l, f, 0);
}

void list_for_each_sync(struct list *l, void (*f)(void *)) {
    return _list_for_each(l, f, 1);
}
