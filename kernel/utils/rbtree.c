#include <utils/rbtree.h>

#define RBTREE_MAX_ALIGN (alignof(max_align_t))

void* rbnode_raw(struct rbnode* node) {
	return (void*)(node->parent_and_flags & ~RBNODE_FLAG_MASK);
}

unsigned long rbnode_flags(struct rbnode* n) {
	return n->parent_and_flags & RBNODE_FLAG_MASK;
}

bool rbnode_is_red(struct rbnode* n) {
	return rbnode_flags(n) & RBNODE_RED;
}

bool rbnode_is_black(struct rbnode* n) {
	return !(rbnode_flags(n) & RBNODE_RED);
}

bool rbnode_is_root(struct rbnode* n) {
	return rbnode_flags(n) & RBNODE_ROOT;
}

struct rbnode* rbnode_leftmost(struct rbnode* node) {
	if (node) {
		while (node->left) {
			node = node->left;
		}
	}

	return node;
}

struct rbnode* rbnode_rightmost(struct rbnode* node) {
	if (node) {
		while (node->right) {
			node = node->right;
		}
	}

	return node;
}

struct rbnode* rbnode_leftdeepest(struct rbnode* node) {
	if (node) {
		for (;;) {
			if (node->left) {
				node = node->left;
			} else if (node->right) {
				node = node->right;
			} else {
				break;
			}
		}
	}

	return node;
}

struct rbnode* rbnode_rightdeepest(struct rbnode* node) {
	if (node) {
		for (;;) {
			if (node->right) {
				node = node->right;
			} else if (node->left) {
				node = node->left;
			} else {
				break;
			}
		}
	}

	return node;
}

struct rbnode* rbnode_next(struct rbnode* node) {
	struct rbnode* parent;

	if (!rbnode_is_linked(node)) {
		return NULL;
	}

	if (node->right) {
		return rbnode_leftmost(node->right);
	}

	while ((parent = rbnode_parent(node)) && node == parent->right) {
		node = parent;
	}

	return parent;
}

struct rbnode* rbnode_prev(struct rbnode* node) {
	struct rbnode* parent;

	if (!rbnode_is_linked(node)) {
		return NULL;
	}

	if (node->left) {
		return rbnode_rightmost(node->left);
	}

	while ((parent = rbnode_parent(node)) && node == parent->left) {
		node = parent;
	}

	return parent;
}

struct rbnode* rbnode_next_postorder(struct rbnode* node) {
	struct rbnode* parent;

	if (!rbnode_is_linked(node)) {
		return NULL;
	}

	parent = rbnode_parent(node);

	if (parent && node == parent->left && parent->right) {
		return rbnode_leftdeepest(parent->right);
	}

	return parent;
}

struct rbnode* rbnode_prev_postorder(struct rbnode* node) {
	struct rbnode* parent;

	if (!rbnode_is_linked(node)) {
		return NULL;
	}

	if (node->right) {
		return node->right;
	}

	if (node->left) {
		return node->left;
	}

	while ((parent = rbnode_parent(node))) {
		if (parent->left && node != parent->left) {
			return parent->left;
		}

		node = parent;
	}

	return NULL;
}

struct rbnode* rbtree_first(struct rbtree* tree) {
	assert(tree);
	return rbnode_leftmost(tree->root);
}

struct rbnode* rbtree_last(struct rbtree* tree) {
	assert(tree);
	return rbnode_rightmost(tree->root);
}

struct rbnode* rbtree_first_postorder(struct rbtree* tree) {
	assert(tree);
	return rbnode_leftdeepest(tree->root);
}

struct rbnode* rbtree_last_postorder(struct rbtree* tree) {
	assert(tree);
	return tree->root;
}

void rbtree_store(struct rbnode** ptr, struct rbnode* addr) {
	*(volatile struct rbnode**)ptr = addr;
}

void rbnode_set_parent_and_flags(struct rbnode* node, struct rbnode* parent,
								 size_t flags) {
	node->parent_and_flags = (size_t)parent | flags;
}

struct rbtree* rbnode_pop_root(struct rbnode* node) {
	struct rbtree* tree = NULL;

	if (rbnode_is_root(node)) {
		tree = rbnode_raw(node);
		node->parent_and_flags = rbnode_flags(node) & ~RBNODE_ROOT;
	}

	return tree;
}

struct rbtree* rbnode_push_root(struct rbnode* node, struct rbtree* tree) {
	if (tree) {
		if (node) {
			node->parent_and_flags =
				(size_t)tree | rbnode_flags(node) | RBNODE_ROOT;
		}

		rbtree_store(&tree->root, node);
	}

	return NULL;
}

void rbnode_swap_child(struct rbnode* old, struct rbnode* new) {
	struct rbnode* parent = rbnode_parent(old);

	if (parent) {
		if (parent->left == old) {
			rbtree_store(&parent->left, new);
		} else {
			rbtree_store(&parent->right, new);
		}
	}
}

void rbtree_move(struct rbtree* to, struct rbtree* from) {
	assert(to->root);

	struct rbtree* tree;

	if (from->root) {
		tree = rbnode_pop_root(from->root);
		assert(tree == from);

		to->root = from->root;
		from->root = NULL;

		rbnode_push_root(to->root, to);
	}
}

void rbtree_paint_terminal(struct rbnode* node) {
	struct rbnode* parent = rbnode_parent(node);
	struct rbnode* g = rbnode_parent(parent);
	struct rbnode* gg = rbnode_parent(g);
	struct rbnode* x;
	struct rbtree* tree;

	assert(rbnode_is_red(parent));
	assert(rbnode_is_black(g));
	assert(parent == g->left || !g->left || rbnode_is_black(g->left));
	assert(parent == g->right || !g->right || rbnode_is_black(g->right));

	if (parent == g->left) {
		if (node == g->right) {
			x = node->left;

			rbtree_store(&parent->right, x);
			rbtree_store(&node->left, parent);

			if (x) {
				rbnode_set_parent_and_flags(x, parent, rbnode_flags(x));
			}

			rbnode_set_parent_and_flags(parent, node, rbnode_flags(parent));
			parent = node;
		}

		x = parent->right;
		tree = rbnode_pop_root(g);

		rbtree_store(&g->left, x);
		rbtree_store(&parent->right, g);

		rbnode_swap_child(g, parent);

		if (x) {
			rbnode_set_parent_and_flags(x, g, rbnode_flags(x) & ~RBNODE_RED);
		}

		rbnode_set_parent_and_flags(parent, gg,
									rbnode_flags(parent) & ~RBNODE_RED);
		rbnode_set_parent_and_flags(g, parent, rbnode_flags(g) | RBNODE_RED);

		rbnode_push_root(parent, tree);
	} else {
		if (node == parent->left) {
			x = node->right;

			rbtree_store(&parent->left, node->right);
			rbtree_store(&node->right, parent);

			if (x) {
				rbnode_set_parent_and_flags(x, parent, rbnode_flags(x));
			}

			rbnode_set_parent_and_flags(parent, node, rbnode_flags(parent));
			parent = node;
		}

		x = parent->left;
		tree = rbnode_pop_root(g);

		rbtree_store(&g->right, x);
		rbtree_store(&parent->left, g);

		rbnode_swap_child(g, parent);

		if (x) {
			rbnode_set_parent_and_flags(x, g, rbnode_flags(x) & ~RBNODE_RED);
		}

		rbnode_set_parent_and_flags(parent, gg,
									rbnode_flags(parent) & ~RBNODE_RED);
		rbnode_set_parent_and_flags(g, parent, rbnode_flags(g) | RBNODE_RED);

		rbnode_push_root(parent, tree);
	}
}

struct rbnode* rbtree_paint_path(struct rbnode* node) {
	struct rbnode* parent;
	struct rbnode* g;
	struct rbnode* u;

	for (;;) {
		parent = rbnode_parent(node);

		if (!parent) {
			rbnode_set_parent_and_flags(node, rbnode_raw(node),
										rbnode_flags(node) & ~RBNODE_RED);
			return NULL;
		} else if (rbnode_is_black(parent)) {
			return NULL;
		}

		g = rbnode_parent(parent);
		u = (parent == g->left) ? g->right : g->left;

		if (!u || !rbnode_is_red(u)) {
			return node;
		}

		rbnode_set_parent_and_flags(parent, g,
									rbnode_flags(parent) & ~RBNODE_RED);
		rbnode_set_parent_and_flags(u, g, rbnode_flags(u) & ~RBNODE_RED);
		rbnode_set_parent_and_flags(g, rbnode_raw(g),
									rbnode_flags(g) | RBNODE_RED);

		node = g;
	}
}

void rbtree_paint(struct rbnode* node) {
	node = rbtree_paint_path(node);

	if (node) {
		rbtree_paint_terminal(node);
	}
}

void rbnode_link(struct rbnode* parent, struct rbnode** link,
				 struct rbnode* node) {
	assert(parent);
	assert(link);
	assert(node);

	assert(link == &parent->left || link == &parent->right);

	rbnode_set_parent_and_flags(node, parent, RBNODE_RED);
	rbtree_store(&node->left, NULL);
	rbtree_store(&node->right, NULL);
	rbtree_store(link, node);

	rbtree_paint(node);
}

void rbtree_add(struct rbtree* tree, struct rbnode* parent,
				struct rbnode** left, struct rbnode* node) {
	assert(parent);
	assert(left);
	assert(node);

	assert(!parent || left == &parent->left || left == &parent->right);
	assert(parent || left == &tree->root);

	rbnode_set_parent_and_flags(node, parent, RBNODE_RED);
	rbtree_store(&node->left, NULL);
	rbtree_store(&node->right, NULL);

	if (parent) {
		rbtree_store(left, node);
	} else {
		rbnode_push_root(node, tree);
	}

	rbtree_paint(node);
}

void rbnode_rebalance_terminal(struct rbnode* parent, struct rbnode* previous) {
	struct rbnode *s, *x, *y, *g;
	struct rbtree* tree;

	if (previous == parent->left) {
		s = parent->right;

		if (rbnode_is_red(s)) {
			tree = rbnode_pop_root(parent);
			g = rbnode_parent(parent);
			x = g->left;

			rbtree_store(&parent->right, x);
			rbtree_store(&s->left, parent);

			rbnode_swap_child(parent, s);

			rbnode_set_parent_and_flags(x, parent,
										rbnode_flags(x) & ~RBNODE_RED);
			rbnode_set_parent_and_flags(s, g, rbnode_flags(x) & ~RBNODE_RED);
			rbnode_set_parent_and_flags(parent, s,
										rbnode_flags(x) | RBNODE_RED);

			rbnode_push_root(s, tree);

			s = x;
		}

		x = s->right;

		if (!x || rbnode_is_black(x)) {
			y = s->left;

			if (!y || rbnode_is_black(y)) {
				assert(rbnode_is_red(parent));
				rbnode_set_parent_and_flags(s, parent,
											rbnode_flags(s) | RBNODE_RED);
				rbnode_set_parent_and_flags(parent, rbnode_parent(parent),
											rbnode_flags(parent) & ~RBNODE_RED);

				return;
			}

			x = y->right;

			rbtree_store(&s->left, y->right);
			rbtree_store(&y->right, s);
			rbtree_store(&parent->right, y);

			if (x) {
				rbnode_set_parent_and_flags(x, s,
											rbnode_flags(x) & ~RBNODE_RED);
			}

			x = s;
			s = y;
		}

		tree = rbnode_pop_root(parent);
		g = rbnode_parent(parent);
		y = s->left;

		rbtree_store(&parent->right, y);
		rbtree_store(&parent->left, parent);

		rbnode_swap_child(parent, s);

		rbnode_set_parent_and_flags(x, s, rbnode_flags(x) & ~RBNODE_RED);

		if (y) {
			rbnode_set_parent_and_flags(y, parent, rbnode_flags(y));
		}

		rbnode_set_parent_and_flags(s, g, rbnode_flags(parent));
		rbnode_set_parent_and_flags(parent, s,
									rbnode_flags(parent) & ~RBNODE_RED);
		rbnode_push_root(s, tree);
	} else {
		s = parent->left;

		if (rbnode_is_red(s)) {
			tree = rbnode_pop_root(parent);
			g = rbnode_parent(parent);
			x = g->right;

			rbtree_store(&parent->left, x);
			rbtree_store(&s->right, parent);

			rbnode_swap_child(parent, s);

			rbnode_set_parent_and_flags(x, parent,
										rbnode_flags(x) & ~RBNODE_RED);
			rbnode_set_parent_and_flags(s, g, rbnode_flags(x) & ~RBNODE_RED);
			rbnode_set_parent_and_flags(parent, s,
										rbnode_flags(x) | RBNODE_RED);

			rbnode_push_root(s, tree);

			s = x;
		}

		x = s->left;

		if (!x || rbnode_is_black(x)) {
			y = s->right;

			if (!y || rbnode_is_black(y)) {
				assert(rbnode_is_red(parent));
				rbnode_set_parent_and_flags(s, parent,
											rbnode_flags(s) | RBNODE_RED);
				rbnode_set_parent_and_flags(parent, rbnode_parent(parent),
											rbnode_flags(parent) & ~RBNODE_RED);

				return;
			}

			x = y->left;

			rbtree_store(&s->right, y->left);
			rbtree_store(&y->left, s);
			rbtree_store(&parent->left, y);

			if (x) {
				rbnode_set_parent_and_flags(x, s,
											rbnode_flags(x) & ~RBNODE_RED);
			}

			x = s;
			s = y;
		}

		tree = rbnode_pop_root(parent);
		g = rbnode_parent(parent);
		y = s->right;

		rbtree_store(&parent->left, y);
		rbtree_store(&parent->right, parent);

		rbnode_swap_child(parent, s);

		rbnode_set_parent_and_flags(x, s, rbnode_flags(x) & ~RBNODE_RED);

		if (y) {
			rbnode_set_parent_and_flags(y, parent, rbnode_flags(y));
		}

		rbnode_set_parent_and_flags(s, g, rbnode_flags(parent));
		rbnode_set_parent_and_flags(parent, s,
									rbnode_flags(parent) & ~RBNODE_RED);
		rbnode_push_root(s, tree);
	}
}

struct rbnode* rbnode_rebalance_path(struct rbnode* parent,
									 struct rbnode** previous) {
	struct rbnode *s, *left_node, *right_node;

	while (parent) {
		s = (*previous == parent->left) ? parent->right : parent->left;

		left_node = s->left;
		right_node = s->right;

		if (rbnode_is_red(s) || (left_node && rbnode_is_red(left_node)) ||
			(right_node && rbnode_is_red(right_node))) {
			return parent;
		}

		rbnode_set_parent_and_flags(s, parent, rbnode_flags(s) | RBNODE_RED);

		if (rbnode_is_red(parent)) {
			rbnode_set_parent_and_flags(parent, rbnode_parent(parent),
										rbnode_flags(parent) & ~RBNODE_RED);
			return NULL;
		}

		*previous = parent;
		parent = rbnode_parent(parent);
	}

	return NULL;
}

void rbnode_rebalance(struct rbnode* node) {
	struct rbnode* previous = NULL;
	node = rbnode_rebalance_path(node, &previous);

	if (node) {
		rbnode_rebalance_terminal(node, previous);
	}
}

void rbnode_unlink_stale(struct rbnode* node) {
	assert(node);
	assert(rbnode_is_linked(node));

	struct rbtree* tree;

	if (!node->left && !node->right) {
		tree = rbnode_pop_root(node);

		rbnode_swap_child(node, NULL);
		rbnode_push_root(NULL, tree);

		if (rbnode_is_black(node)) {
			rbnode_rebalance(rbnode_parent(node));
		}
	} else if (!node->left && node->right) {
		tree = rbnode_pop_root(node);

		rbnode_swap_child(node, node->right);
		rbnode_set_parent_and_flags(node->right, rbnode_parent(node),
									rbnode_flags(node->right) & ~RBNODE_RED);
		rbnode_push_root(node->right, tree);
	} else if (node->left && !node->right) {
		tree = rbnode_pop_root(node);

		rbnode_swap_child(node, node->left);
		rbnode_set_parent_and_flags(node->left, rbnode_parent(node),
									rbnode_flags(node->left) & ~RBNODE_RED);
		rbnode_push_root(node->left, tree);
	} else {
		struct rbnode *s, *p, *c, *next = NULL;

		tree = rbnode_pop_root(node);
		s = node->right;

		if (!s->left) {
			p = s;
			c = s->right;
		} else {
			s = rbnode_leftmost(s);
			p = rbnode_parent(s);
			c = s->right;

			rbtree_store(&p->left, c);
			rbtree_store(&s->right, node->right);

			rbnode_set_parent_and_flags(node->right, s,
										rbnode_flags(node->right));
		}

		rbtree_store(&s->left, node->left);

		rbnode_set_parent_and_flags(node->left, s, rbnode_flags(node->left));

		if (c) {
			rbnode_set_parent_and_flags(c, p, rbnode_flags(c) & ~RBNODE_RED);
		} else {
			next = rbnode_is_black(s) ? p : NULL;
		}

		if (rbnode_is_red(node)) {
			rbnode_set_parent_and_flags(s, rbnode_parent(node),
										rbnode_flags(s) | RBNODE_RED);
		} else {
			rbnode_set_parent_and_flags(s, rbnode_parent(node),
										rbnode_flags(s) & ~RBNODE_RED);
		}

		rbnode_swap_child(node, s);
		rbnode_push_root(s, tree);

		if (next) {
			rbnode_rebalance(next);
		}
	}
}