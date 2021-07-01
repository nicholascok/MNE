#ifndef __CAOIMH_HUFFMAN_H__
#define __CAOIMH_HUFFMAN_H__

#include <malloc.h>

/* structs */
struct htree_node {
	struct htree_node* pnode;
	struct htree_node* lnode;
	struct htree_node* rnode;
	int freq, val;
};

struct hf_heap {
	struct htree_node** elems;
	int max_sz, cur_sz;
};

/* declarations */
struct hf_heap* hf_heap_init(int _sz);
struct hf_heap* hf_heap_free(struct hf_heap* _h);
void hf_heap_heapify(struct hf_heap* _h, int _i);
void hf_heap_insert(struct hf_heap* _h, struct htree_node* _node);
struct htree_node* hf_heap_extract(struct hf_heap* _h);

struct htree_node* hf_generate_tree_from_ascii(char* message);

/* definitions */
struct hf_heap* hf_heap_init(int _sz) {
	struct hf_heap* h = malloc(sizeof(struct hf_heap));
	h->max_sz = _sz, h->cur_sz = 0;
	h->elems = malloc(sizeof(void*) * _sz);
	return h;
}

struct hf_heap* hf_heap_free(struct hf_heap* _h) {
	free(_h->elems);
	free(_h);
}

void hf_heap_insert(struct hf_heap* _h, struct htree_node* _node) {
	_h->elems[_h->cur_sz] = _node;
	for (int i = _h->cur_sz++; _h->elems[i]->freq < _h->elems[(i - 1) / 2]->freq;) {
		SWAP_PTR(_h->elems[i], _h->elems[(i - 1) / 2]);
		i = (i - 1) / 2;
	}
}

void hf_heap_heapify(struct hf_heap* _h, int _i) {
	int lft = 2 * _i + 1, rgt = 2 * _i + 2, min = _i;
	if (!(_h->elems[lft] && _h->elems[rgt])) return;
	if (_h->elems[lft]->freq < _h->elems[min]->freq)
		min = lft;
	if (_h->elems[rgt]->freq < _h->elems[min]->freq)
		min = rgt;
	if (min - _i) {
		SWAP_PTR(_h->elems[_i], _h->elems[min]);
		hf_heap_heapify(_h, min);
	}
}

struct htree_node* hf_heap_extract(struct hf_heap* _h) {
	if (_h->cur_sz <= 0) return NULL;
	struct htree_node* ret = _h->elems[0];
	_h->elems[0] = _h->elems[--_h->cur_sz];
	_h->elems[_h->cur_sz] = NULL;
	hf_heap_heapify(_h, 0);
	return ret;
}

struct htree_node* hf_generate_tree_from_ascii(char* message) {
	char sfreq_char[] = {[0xFF] = 0};
	struct hf_heap* h = hf_heap_init(1024);
	
	for (int i = 0; message[i] != 0; i++)
		sfreq_char[message[i]]++;
	
	for (int i = 0; i < sizeof(sfreq_char); i++)
		if (sfreq_char[i]) {
			struct htree_node* node = malloc(sizeof(struct htree_node));
			node->freq = sfreq_char[i];
			node->val = i;
			hf_heap_insert(h, node);
		}
	
	while (h->cur_sz > 1) {
		struct htree_node* lnode = hf_heap_extract(h);
		struct htree_node* rnode = hf_heap_extract(h);
		struct htree_node* pnode = malloc(sizeof(struct htree_node));
		pnode->lnode = lnode, pnode->rnode = rnode;
		lnode->pnode = pnode, rnode->pnode = pnode;
		pnode->freq = lnode->freq + rnode->freq;
		hf_heap_insert(h, pnode);
	}
	struct htree_node* root = hf_heap_extract(h);
	hf_heap_free(h);
	return root;
}

#endif