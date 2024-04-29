#ifndef QAMAR_LIST_H
#define QAMAR_LIST_H

struct list_node {
	struct list_node *next;
	void *data;
};

struct list {
	struct list_node *head;
};

void
qamar_list_insert(struct list *list, void *data);

void
qamar_list_free(struct list *list, void (*dtor)(void*));

#endif // QAMAR_LIST_H