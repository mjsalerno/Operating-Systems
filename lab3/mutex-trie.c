/* A (reverse) trie with trie-wide mutual exclusion. */

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "trie.h"

struct trie_node {
    struct trie_node *next; /* parent list */
    unsigned int strlen; /* Length of the key */
    int32_t ip4_address; /* 4 octets */
    struct trie_node *children; /* Sorted list of children */
    char key[64]; /* Up to 64 chars */
};

static struct trie_node * root = NULL;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

struct trie_node * new_leaf(const char *string, size_t strlen, int32_t ip4_address) {
    printf("------------new_leaf\n");
    struct trie_node *new_node = malloc(sizeof (struct trie_node));
    
    assert(strlen < 64);
    assert(strlen > 0);
    new_node->next = NULL;
    new_node->strlen = strlen;
    strncpy(new_node->key, string, strlen);
    new_node->key[strlen] = '\0';
    new_node->ip4_address = ip4_address;
    new_node->children = NULL;


    printf("------------new_leaf done\n");
    return new_node;
}

int compare_keys(const char *string1, int len1, const char *string2, int len2, int *pKeylen) {
    int keylen, offset1, offset2, result;

    keylen = len1 < len2 ? len1 : len2;
    offset1 = len1 - keylen;
    offset2 = len2 - keylen;
    assert(keylen > 0);
    if (pKeylen)
        *pKeylen = keylen;

    result = strncmp(&string1[offset1], &string2[offset2], keylen); 
    return result;
}

void init(int numthreads) {
    root = NULL;
}

/* Recursive helper function.
 * Returns a pointer to the node if found.
 * Stores an optional pointer to the 
 * parent, or what should be the parent if not found.
 * 
 */
struct trie_node *
_search(struct trie_node *node, const char *string, size_t strlen) {

    int keylen, cmp, rc;

    // First things first, check if we are NULL 
    rc = pthread_mutex_lock(&mutex);
    assert(rc == 0);
    printf("------------_search locked\n");
    if (node == NULL) {
        rc = pthread_mutex_unlock(&mutex);
        assert(rc == 0);
        printf("------------_search unlocking\n");
        return NULL;  
    } 

    assert(node->strlen < 64);

    // See if this key is a substring of the string passed in
    cmp = compare_keys(node->key, node->strlen, string, strlen, &keylen);
    if (cmp == 0) {
        // Yes, either quit, or recur on the children

        // If this key is longer than our search string, the key isn't here
        if (node->strlen > keylen) {
            rc = pthread_mutex_unlock(&mutex);
            assert(rc == 0);
            printf("------------_search unlocking\n");
            return NULL;
        } else if (strlen > keylen) {
            // Recur on children list
            rc = pthread_mutex_unlock(&mutex);
            assert(rc == 0);
            printf("------------_search unlocking\n");
            return _search(node->children, string, strlen - keylen);
        } else {
            assert(strlen == keylen);
            rc = pthread_mutex_unlock(&mutex);
            assert(rc == 0);
            printf("------------_search unlocking\n");
            return node;
        }

    } else if (cmp < 0) {
        // No, look right (the node's key is "less" than the search key)
        rc = pthread_mutex_unlock(&mutex);
        assert(rc == 0);
        printf("------------_search unlocking\n");
        return _search(node->next, string, strlen);
    } else {
        // Quit early
        rc = pthread_mutex_unlock(&mutex);
        assert(rc == 0);
        printf("------------_search unlocking\n");
        return 0;
    }

}

int search(const char *string, size_t strlen, int32_t *ip4_address) {
    int rc, result;

    struct trie_node *found;

    // Skip strings of length 0
    if (strlen == 0){
        return 0;
    }

    found = _search(root, string, strlen);

    printf("------------search locking\n");
    rc = pthread_mutex_lock(&mutex);
    assert(rc == 0);

    if (found && ip4_address)
        *ip4_address = found->ip4_address;

    result = (found != NULL);
    
     rc = pthread_mutex_unlock(&mutex);
    assert(rc == 0);
    printf("------------search unlocked\n");

    return result;
}

/* Recursive helper function */
int _insert(const char *string, size_t strlen, int32_t ip4_address,
        struct trie_node *node, struct trie_node *parent, struct trie_node *left) {

    int cmp, keylen;

    // First things first, check if we are NULL 
    assert(node != NULL);
    assert(node->strlen < 64);

    // Take the minimum of the two lengths
    cmp = compare_keys(node->key, node->strlen, string, strlen, &keylen);
    if (cmp == 0) {
        // Yes, either quit, or recur on the children

        // If this key is longer than our search string, we need to insert
        // "above" this node
        if (node->strlen > keylen) {
            struct trie_node *new_node;

            assert(keylen == strlen);
            assert((!parent) || parent->children == node);

            new_node = new_leaf(string, strlen, ip4_address);
            node->strlen -= keylen;
            new_node->children = node;

            assert((!parent) || (!left));

            if (parent) {
                parent->children = new_node;
            } else if (left) {
                left->next = new_node;
            } else if ((!parent) || (!left)) {
                root = new_node;
            }
            return 1;

        } else if (strlen > keylen) {

            if (node->children == NULL) {
                // Insert leaf here
                struct trie_node *new_node = new_leaf(string, strlen - keylen, ip4_address);
                node->children = new_node;
                return 1;
            } else {
                // Recur on children list, store "parent" (loosely defined)
                return _insert(string, strlen - keylen, ip4_address,
                        node->children, node, NULL);
            }
        } else {
            assert(strlen == keylen);
            if (node->ip4_address == 0) {
                node->ip4_address = ip4_address;
                return 1;
            } else {
                return 0;
            }
        }

    } else {
        /* Is there any common substring? */
        int i, cmp2, keylen2, overlap = 0;
        for (i = 1; i < keylen; i++) {
            cmp2 = compare_keys(&node->key[i], node->strlen - i,
                    &string[i], strlen - i, &keylen2);
            assert(keylen2 > 0);
            if (cmp2 == 0) {
                overlap = 1;
                break;
            }
        }

        if (overlap) {
            // Insert a common parent, recur
            struct trie_node *new_node = new_leaf(&string[i], strlen - i, ip4_address);
            int diff = node->strlen - i;
            assert((node->strlen - diff) > 0);
            node->strlen -= diff;
            new_node->children = node;
            assert((!parent) || (!left));

            if (parent) {
                parent->children = new_node;
            } else if (left) {
                left->next = new_node;
            } else if ((!parent) && (!left)) {
                root = new_node;
            }

            return _insert(string, i, ip4_address,
                    node, new_node, NULL);

        } else if (cmp > 0) {
            if (node->next == NULL) {
                // Insert here
                struct trie_node *new_node = new_leaf(string, strlen, ip4_address);
                node->next = new_node;
                return 1;
            } else {
                // No, recur right (the node's key is "greater" than  the search key)
                return _insert(string, strlen, ip4_address, node->next, NULL, node);
            }
        } else {
            // Insert here
            struct trie_node *new_node = new_leaf(string, strlen, ip4_address);
            node->next = new_node;
            return 1;
        }
    }
}

int insert(const char *string, size_t strlen, int32_t ip4_address) {
    printf("------------insert locking\n");
    int rc, result;
    rc = pthread_mutex_lock(&mutex);
    assert(rc == 0);
    printf("------------insert locked\n");
    // Skip strings of length 0
    if (strlen == 0){
        rc = pthread_mutex_unlock(&mutex);
        assert(rc == 0);
        return 0;
    }

    /* Edge case: root is null */
    if (root == NULL) {
        root = new_leaf(string, strlen, ip4_address);
        rc = pthread_mutex_unlock(&mutex);
        assert(rc == 0);
        printf("------------insert unlocked\n");
        return 1;
    }
    
    rc = pthread_mutex_unlock(&mutex);
    assert(rc == 0);
    printf("------------insert unlocked\n");
    result = _insert(string, strlen, ip4_address, root, NULL, NULL);

    return result;
}

/* Recursive helper function.
 * Returns a pointer to the node if found.
 * Stores an optional pointer to the 
 * parent, or what should be the parent if not found.
 * 
 */
struct trie_node *
_delete(struct trie_node *node, const char *string,
        size_t strlen) {
    int keylen, offset1, offset2, cmp;

    // First things first, check if we are NULL 
    if (node == NULL) return NULL;

    assert(node->strlen < 64);

    // Take the minimum of the two lengths
    keylen = node->strlen < strlen ? node->strlen : strlen;
    offset1 = node->strlen - keylen;
    offset2 = strlen - keylen;

    // See if this key is a substring of the string passed in
    cmp = strncmp(&node->key[offset1], &string[offset2], keylen);
    if (cmp == 0) {
        // Yes, either quit, or recur on the children

        // If this key is longer than our search string, the key isn't here
        if (node->strlen > keylen) {
            return NULL;
        } else if (strlen > keylen) {
            struct trie_node *found = _delete(node->children, string, strlen - keylen);
            if (found) {
                /* If the node doesn't have children, delete it.
                 * Otherwise, keep it around to find the kids */
                if (found->children == NULL && found->ip4_address == 0) {
                    if (node->next == found)
                        node->next = found->next;
                    else
                        node->children = found->next;
                    free(found);
                }
                return node; /* Recursively delete needless interior nodes */
            } else
                return NULL;
        } else {
            assert(strlen == keylen);

            /* We found it! Clear the ip4 address and return. */
            if (node->ip4_address) {
                node->ip4_address = 0;
                return node;
            } else {
                /* Just an interior node with no value */
                return NULL;
            }
        }

    } else if (cmp < 0) {
        // No, look right (the node's key is "less" than  the search key)
        struct trie_node *found = _delete(node->next, string, strlen);
        if (found) {
            /* If the node doesn't have children, delete it.
             * Otherwise, keep it around to find the kids */
            if (found->children == NULL && found->ip4_address) {
                if (node->next == found)
                    node->next = found->next;
                else
                    node->children = found->next;
                free(found);
            }
            return node; /* Recursively delete needless interior nodes */
        }
        return NULL;
    } else {
        // Quit early
        return NULL;
    }

}

int delete(const char *string, size_t strlen) {
    printf("------------del locking\n");
    int result, rc;
    rc = pthread_mutex_lock(&mutex);
    printf("------------del locked\n");
    assert(rc == 0);

    // Skip strings of length 0
    if (strlen == 0){
        rc = pthread_mutex_unlock(&mutex);
        assert(rc == 0);
        return 0;
    }

    result = (NULL != _delete(root, string, strlen));

    rc = pthread_mutex_unlock(&mutex);
    assert(rc == 0);
    printf("------------del unlocked\n");
    return result;
}

void _print(struct trie_node *node) {
    printf("Node at %p.  Key %.*s, IP %d.  Next %p, Children %p\n",
            node, node->strlen, node->key, node->ip4_address, node->next, node->children);
    if (node->children)
        _print(node->children);
    if (node->next)
        _print(node->next);
}

void print() {
    int rc = pthread_mutex_lock(&mutex);
    assert(rc == 0);
    /* Do a simple depth-first search */
    _print(root);

    rc = pthread_mutex_unlock(&mutex);
    assert(rc == 0);
}
