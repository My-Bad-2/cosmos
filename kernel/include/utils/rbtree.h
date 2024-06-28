#ifndef UTILS_RBTREE_H
#define UTILS_RBTREE_H

#include <assert.h>
#include <memory/addr.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum rbnode_type { RBNODE_RED = 1, RBNODE_ROOT, RBNODE_FLAG_MASK };

struct rbnode {
	size_t parent_and_flags;

	struct rbnode* left;
	struct rbnode* right;
};

#define RBNODE_INITIALIZER(var)                                                \
	((struct rbnode){.parent_and_flags = (size_t) & (var)})

struct rbnode* rbnode_leftmost(struct rbnode* node);
struct rbnode* rbnode_rightmost(struct rbnode* node);
struct rbnode* rbnode_leftdeepest(struct rbnode* node);
struct rbnode* rbnode_rightdeepest(struct rbnode* node);
struct rbnode* rbnode_next(struct rbnode* node);
struct rbnode* rbnode_prev(struct rbnode* node);
struct rbnode* rbnode_next_postorder(struct rbnode* node);
struct rbnode* rbnode_prev_postorder(struct rbnode* node);

void rbnode_link(struct rbnode* parent, struct rbnode** link,
				 struct rbnode* node);
void rbnode_unlink_stale(struct rbnode* node);

struct rbtree {
	struct rbnode* root;
};

#define RBTREE_INITIALIZER ((struct rbtree){})

struct rbnode* rbtree_first(struct rbtree* tree);
struct rbnode* rbtree_last(struct rbtree* tree);
struct rbnode* rbtree_first_postorder(struct rbtree* tree);
struct rbnode* rbtree_last_postorder(struct rbtree* tree);

void rbtree_move(struct rbtree* to, struct rbtree* from);
void rbtree_add(struct rbtree* tree, struct rbnode* parent,
				struct rbnode** left, struct rbnode* node);

static inline void rbnode_init(struct rbnode* node) {
	*node = RBNODE_INITIALIZER(*node);
}

#define rbnode_entry(tree_node, type_of_container, member_name)                \
	((type_of_container*)(void*)(((size_t)(void*)(tree_node)                   \
									  ?: offsetof(type_of_container,           \
												  member_name)) -              \
								 offsetof(type_of_container, member_name)))

static inline struct rbnode* rbnode_parent(struct rbnode* node) {
	return (node->parent_and_flags & RBNODE_ROOT)
			   ? NULL
			   : (void*)(node->parent_and_flags & ~RBNODE_FLAG_MASK);
}

static inline bool rbnode_is_linked(struct rbnode* node) {
	return node && rbnode_parent(node) != node;
}

static inline void rbnode_unlink(struct rbnode* node) {
	if (rbnode_is_linked(node)) {
		rbnode_unlink_stale(node);
		rbnode_init(node);
	}
}

static inline void rbtree_init(struct rbtree* tree) {
	*tree = RBTREE_INITIALIZER;
}

static inline bool rbtree_is_empty(struct rbtree* tree) {
	return tree->root == NULL;
}

typedef int (*rb_compare_func)(struct rbtree* tree, void* key,
							   struct rbnode* node);

static inline struct rbnode*
rbtree_find_node(struct rbtree* tree, rb_compare_func func, const void* key) {
	assert(tree);
	assert(func);

	struct rbnode* iter = tree->root;

	while (iter) {
		int result = func(tree, (void*)key, iter);

		if (result == 0) {
			return iter;
		}

		if (result < 0) {
			iter = iter->right;
		} else {
			iter = iter->left;
		}
	}

	return NULL;
}

#define rbtree_find_entry(tree, func, key, type_of_structure, member_name)     \
	rbnode_entry(rbtree_find_node((tree), (func), (key)), (type_of_structure), \
				 (member_name))

static inline struct rbnode** rbtree_find_slot(struct rbtree* tree,
											   rb_compare_func func,
											   const void* key,
											   struct rbnode** parent) {
	assert(tree);
	assert(func);
	assert(parent);

	struct rbnode** iter = &tree->root;
	*parent = NULL;

	while (*iter) {
		int result = func(tree, (void*)key, *iter);
		*parent = *iter;

		if (result == 0) {
			return NULL;
		}

		if (result < 0) {
			iter = &(*iter)->left;
		} else {
			iter = &(*iter)->right;
		}
	}

	return iter;
}

#define rbtree_for_each(iter, tree)                                            \
	for (iter = rbtree_first(tree); iter; iter = rbnode_next(iter))

#define rbtree_for_each_entry(iter, tree, member)                              \
	for (iter = rbnode_entry(rbtree_first(tree), __typeof__(*iter), member);   \
		 iter; iter = rbnode_entry(rbnode_next(&iter->member),                 \
								   __typeof__(*iter), member))

#define rbtree_for_eachsafe(iter, safe, tree)                                  \
	for (iter = rbtree_first(tree), safe = rbnode_next(iter); iter;            \
		 iter = safe, safe = rbnode_next(safe))

#define rbtree_for_each_entrysafe(iter, safe, tree, member)                    \
	for (iter = rbnode_entry(rbtree_first(tree), __typeof__(*iter), member),   \
		safe = iter ? rbnode_entry(rbnode_next(&iter->member),                 \
								   __typeof__(*iter), member)                  \
					: NULL;                                                    \
		 iter;                                                                 \
		 iter = safe, safe = safe ? rbnode_entry(rbnode_next(&safe->member),   \
												 __typeof__(*iter), member)    \
								  : NULL)

#define rbtree_for_each_postorder(iter, tree)                                  \
	for (iter = rbtree_first_postorder(tree); iter;                            \
		 iter = rbnode_next_postorder(iter))

#define rbtree_for_each_entry_postorder(iter, tree, member)                    \
	for (iter = rbnode_entry(rbtree_first_postorder(tree), __typeof__(*iter),  \
							 member);                                          \
		 iter; iter = rbnode_entry(rbnode_next_postorder(&iter->member),       \
								   __typeof__(*iter), member))

#define rbtree_for_eachsafe_postorder(iter, safe, tree)                        \
	for (iter = rbtree_first_postorder(tree),                                  \
		safe = rbnode_next_postorder(iter);                                    \
		 iter; iter = safe, safe = rbnode_next_postorder(safe))

#define rbtree_for_each_entrysafe_postorder(iter, safe, tree, member)          \
	for (iter = rbnode_entry(rbtree_first_postorder(tree), __typeof__(*iter),  \
							 member),                                          \
		safe = iter ? rbnode_entry(rbnode_next_postorder(&iter->member),       \
								   __typeof__(*iter), member)                  \
					: NULL;                                                    \
		 iter; iter = safe,                                                    \
		safe = safe ? rbnode_entry(rbnode_next_postorder(&safe->member),       \
								   __typeof__(*iter), member)                  \
					: NULL)

#define rbtree_for_eachsafe_postorder_unlink(iter, safe, tree)                 \
	for (iter = rbtree_first_postorder(tree),                                  \
		safe = rbnode_next_postorder(iter);                                    \
		 iter ? ((*iter = RBNODE_INIT(*iter)), 1)                              \
			  : (((tree)->root = NULL), 0);                                    \
		 iter = safe, safe = rbnode_next_postorder(safe))

#define rbtree_for_each_entrysafe_postorder_unlink(iter, safe, tree, member)   \
	for (iter = rbnode_entry(rbtree_first_postorder(tree), __typeof__(*iter),  \
							 member),                                          \
		safe = iter ? rbnode_entry(rbnode_next_postorder(&iter->member),       \
								   __typeof__(*iter), member)                  \
					: NULL;                                                    \
		 iter ? ((iter->member = RBNODE_INIT(iter->member)), 1)                \
			  : (((tree)->root = NULL), 0);                                    \
		 iter = safe,                                                          \
		safe = safe ? rbnode_entry(rbnode_next_postorder(&safe->member),       \
								   __typeof__(*iter), member)                  \
					: NULL)

#endif	// UTILS_RBTREE_H