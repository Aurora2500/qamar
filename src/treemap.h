#ifndef QAMAR_TREEMAP_H
#define QAMAR_TREEMAP_H

#include <stdbool.h>
#include <stdint.h>

enum treemap_color {
	RED,
	BLACK
};

struct treemap_node
{
	struct treemap_node *left;
	struct treemap_node *right;
	struct treemap_node *parent;
	enum treemap_color color;
	void *key;
	intptr_t value;
};

struct treemap {
	struct treemap_node *root;
};

void
qamar_treemap_insert(struct treemap *map, void *key, intptr_t value);

bool
qamar_treemap_get(struct treemap *map, void *key, intptr_t* value);

void
qamar_treemap_free(struct treemap *map, void* userdata, void (*dtor)(void*, intptr_t, void*));




#endif // QAMAR_TREEMAP_H