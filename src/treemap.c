#include "treemap.h"
#include <stddef.h>
#include <stdlib.h>

static void
qamar_treemap_rotate_left(struct treemap_node *node)
{
	struct treemap_node *right = node->right;
	node->right = right->left;
	if(right->left != NULL) {
		right->left->parent = node;
	}
	right->parent = node->parent;
	if(node->parent == NULL) {
		node->parent = right;
	} else if(node == node->parent->left) {
		node->parent->left = right;
	} else {
		node->parent->right = right;
	}
	right->left = node;
	node->parent = right;
}

static void
qamar_treemap_rotate_right(struct treemap_node *node)
{
	struct treemap_node *left = node->left;
	node->left = left->right;
	if(left->right != NULL) {
		left->right->parent = node;
	}
	left->parent = node->parent;
	if(node->parent == NULL) {
		node->parent = left;
	} else if(node == node->parent->right) {
		node->parent->right = left;
	} else {
		node->parent->left = left;
	}
	left->right = node;
	node->parent = left;

}

static void
qamar_treemap_fix(struct treemap *map, struct treemap_node *node)
{
	while(node->parent != NULL && node->parent->color == RED) {
		if(node->parent == node->parent->parent->left) {
			struct treemap_node *uncle = node->parent->parent->right;
			if(uncle != NULL && uncle->color == RED) {
				// Case 1
				node->parent->color = BLACK;
				uncle->color = BLACK;
				node->parent->parent->color = RED;
				node = node->parent->parent;
			} else {
				if(node == node->parent->right) {
					// case 2
					node = node->parent;
					qamar_treemap_rotate_left(node);
				}
				// case 3
				node->parent->color = BLACK;
				node->parent->parent->color = RED;
				qamar_treemap_rotate_right(node->parent->parent);
			}
		} else {
			struct treemap_node *uncle = node->parent->parent->left;
			if(uncle != NULL && uncle->color == RED) {
				// Case 1
				node->parent->color = BLACK;
				uncle->color = BLACK;
				node->parent->parent->color = RED;
				node = node->parent->parent;
			} else {
				if(node == node->parent->left) {
					// case 2
					node = node->parent;
					qamar_treemap_rotate_right(node);
				}
				// case 3
				node->parent->color = BLACK;
				node->parent->parent->color = RED;
				qamar_treemap_rotate_left(node->parent->parent);
			}
		}
	}
	map->root->color = BLACK;
}

void
qamar_treemap_insert(struct treemap *map, void *key, intptr_t value) {
	struct treemap_node *node = malloc(sizeof(struct treemap_node));
	node->key = key;
	node->value = value;
	node->left = NULL;
	node->right = NULL;
	node->color = RED;
	
	struct treemap_node *parent = NULL;
	struct treemap_node *current = map->root;
	while (current != NULL) {
		parent = current;
		if (node->key < current->key) {
			current = current->left;
		} else {
			current = current->right;
		}
	}
	node->parent = parent;
	if (parent == NULL) {
		map->root = node;
		node->color = BLACK;
	} else if (node->key < parent->key) {
		parent->left = node;
	} else {
		parent->right = node;
	}
	qamar_treemap_fix(map, node);
}

bool
qamar_treemap_get(struct treemap *map, void *key, intptr_t* value) {
	struct treemap_node *node = map->root;
	while (node != NULL) {
		if(node->key == key) {
			*value = node->value;
			return true;
		}
		if (node->key < key) {
			node = node->right;
		} else {
			node = node->left;
		}
	}
	return false;
}

static void
qamar_treemap_node_free(struct treemap_node *node, void* userdata, void (*dtor)(void*, intptr_t, void*)) {
	if(node == NULL) {
		return;
	}
	qamar_treemap_node_free(node->left, userdata, dtor);
	
	qamar_treemap_node_free(node->right, userdata, dtor);
	if(dtor != NULL) {
		dtor(node->key, node->value, userdata);
	}
	free(node);
}

void
qamar_treemap_free(struct treemap *map, void* userdata, void (*dtor)(void*, intptr_t, void*)) {
	qamar_treemap_node_free(map->root, userdata, dtor);
}