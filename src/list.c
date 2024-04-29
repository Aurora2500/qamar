#include "list.h"

#include <stdlib.h>

void
qamar_list_insert(struct list *list, void *data) {
	struct list_node *node = malloc(sizeof(struct list_node));
	node->data = data;
	node->next = list->head;
	list->head = node;
}

void
qamar_list_free(struct list *list, void (*dtor)(void*)) {
	struct list_node *node = list->head;
	while (node) {
		struct list_node *next = node->next;
		dtor(node->data);
		free(node);
		node = next;
	}
}