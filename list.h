struct _list_node {
    struct _list_node *next;
    struct _list_node *prev;
    void *data;
};

struct list {
    struct _list_node head;
    struct _list_node tail;
    int length;
};

void list_init(struct list *l);

void list_add(struct list *l, void *data);

void *get_first(struct list *l, const void *prop, int (*recognize)(const void *, void *));

void *list_remove_first(struct list *l, const void *prop, int (*recognize)(const void *, void *));

int list_remove_all(struct list *l, const void *prop, int (*recognize)(const void *, void *));

int list_contains(struct list *l, const void *prop, int (*recognize)(const void *, void *));

void list_for_each(struct list *l, void (*f)(void *));
