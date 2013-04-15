/* A simple, (reverse) trie.  Only for use with 1 thread. */

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "trie.h"
#define DEBUG_MUTEX

#ifdef DEBUG_MUTEX
#define PRINT(M, S, ...) printf("---File: %s Line: %3d Thread: %u Method: %s %s\n", __FILE__, __LINE__, (int)pthread_self(), (M), (S));
#define PRINT_LOCK(M, S, L, Line, at, ...) printf("---Line: %3d T: %u LH: %d AT: %d : %s %s ", (Line), (int)pthread_self(), (L), (at), (M), (S)); printf(__VA_ARGS__);printf("\n");
#else
#define PRINT(M, S) 
#endif

struct trie_node {
    struct trie_node *next; /* parent list */
    unsigned int strlen; /* Length of the key */
    int32_t ip4_address; /* 4 octets */
    struct trie_node *children; /* Sorted list of children */
    char key[64]; /* Up to 64 chars */
    pthread_mutex_t mutex;
};

static struct trie_node * root = NULL;
static pthread_mutex_t rootMutex = PTHREAD_MUTEX_INITIALIZER;

void lock(struct trie_node *node, char *function, char *var, int line, int *locksHeld){
    int rv;
    assert(*locksHeld >= 0);
    PRINT_LOCK(function,var, *locksHeld, line, (int)node,"locking")
    rv = pthread_mutex_lock(&(node->mutex));
    (*locksHeld)++;
    PRINT_LOCK(function,var, *locksHeld, line, (int)node,"locked")
    assert(rv == 0);
}

void unlock(struct trie_node *node, char *function, char *var, int line, int *locksHeld){
    int rv;
    assert(*locksHeld >= 0);
    PRINT_LOCK(function,var, *locksHeld, line, (int)node, "unlocking")
    rv = pthread_mutex_unlock(&(node->mutex));
    (*locksHeld)--;
    if(*locksHeld < 0) locksHeld = 0;
    PRINT_LOCK(function,var, *locksHeld, line, (int)node,"unlocked")
    assert(rv == 0);
}

struct trie_node * new_leaf(const char *string, size_t strlen, int32_t ip4_address) {
    struct trie_node *new_node = malloc(sizeof (struct trie_node));
    if (!new_node) {
        printf("WARNING: Node memory allocation failed.  Results may be bogus.\n");
        return NULL;
    }
    assert(strlen < 64);
    assert(strlen > 0 || root == NULL);
    new_node->next = NULL;
    new_node->strlen = strlen;
    strncpy(new_node->key, string, strlen);
    new_node->key[strlen] = '\0';
    new_node->ip4_address = ip4_address;
    new_node->children = NULL;
    pthread_mutex_init( &(new_node->mutex), NULL);

    return new_node;
}

int compare_keys(const char *string1, int len1, const char *string2, int len2, int *pKeylen) {
    int keylen, offset1, offset2;
    keylen = len1 < len2 ? len1 : len2;
    offset1 = len1 - keylen;
    offset2 = len2 - keylen;
    assert(keylen > 0);
    if (pKeylen)
        *pKeylen = keylen;
    return strncmp(&string1[offset1], &string2[offset2], keylen);
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
_search(struct trie_node *node, const char *string, size_t strlen, int *locksHeld) {

    int keylen, cmp;

    // First things first, check if we are NULL k
    if (node == NULL) return NULL;
    assert(node->strlen < 64);

    // See if this key is a substring of the string passed in
    cmp = compare_keys(node->key, node->strlen, string, strlen, &keylen);
    if (cmp == 0) {
        // Yes, either quit, or recur on the children

        // If this key is longer than our search string, the key isn't here
        if (node->strlen > keylen) {
            unlock(node, "_search", "node", __LINE__, locksHeld);
            return NULL;
        } else if (strlen > keylen) {
            // Recur on children list
            if(node->children != NULL)
                lock(node->children, "_search", "node.children", __LINE__, locksHeld);
            unlock(node, "_search", "node", __LINE__, locksHeld);
            return _search(node->children, string, strlen - keylen, locksHeld);
        } else {
            assert(strlen == keylen);
            unlock(node, "_search", "node", __LINE__, locksHeld);
            return node;
        }

    } else if (cmp < 0) {
        // No, look right (the node's key is "less" than the search key)
        if(node->next != NULL)
            lock(node->next, "_search", "node.next", __LINE__, locksHeld);
        
        struct trie_node *tri = _search(node->next, string, strlen, locksHeld);

        unlock(node, "_search", "node", __LINE__, locksHeld);

        return tri;
    } else {
        // Quit early
        unlock(node, "_search", "node", __LINE__, locksHeld);
        return 0;
    }

}

int search(const char *string, size_t strlen, int32_t *ip4_address) {
    struct trie_node *found;
    int locksHeld = 0;

    // Skip strings of length 0
    if (strlen == 0 || root == NULL)
        return 0;

    lock(root, "search", "root", __LINE__, &locksHeld);

    found = _search(root, string, strlen, &locksHeld);

    if (found && ip4_address)
        *ip4_address = found->ip4_address;

    return (found != NULL);
}

/* Recursive helper function */
int _insert(const char *string, size_t strlen, int32_t ip4_address,
        struct trie_node *node, struct trie_node *parent, struct trie_node *left, int *locksHeld) {
    PRINT("_insert", "entered the _insert method")

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
            unlock(node, "_insert", "node", __LINE__, locksHeld);
            return 1;

        } else if (strlen > keylen) {

            if (node->children == NULL) {
                // Insert leaf here                
                struct trie_node *new_node = new_leaf(string, strlen - keylen, ip4_address);
                node->children = new_node;
                unlock(node, "_insert", "node", __LINE__, locksHeld);
                return 1;
            } else {
                // Recur on children list, store "parent" (loosely defined)
                lock(node->children, "_insert", "node.children", __LINE__, locksHeld);
                unlock(node, "_insert", "node", __LINE__, locksHeld);                
                return _insert(string, strlen - keylen, ip4_address,
                        node->children, node, NULL, locksHeld);
            }
        } else {
            assert(strlen == keylen);
            if (node->ip4_address == 0) {
                node->ip4_address = ip4_address;
                unlock(node, "_insert", "node", __LINE__, locksHeld);
                return 1;
            } else {
                //TODO: SQUATTING
                unlock(node, "_insert", "node", __LINE__, locksHeld);
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
            struct trie_node *new_node = new_leaf(&string[i], strlen - i, 0);
            int diff = node->strlen - i;
            assert((node->strlen - diff) > 0);
            node->strlen -= diff;
            new_node->children = node;
            assert((!parent) || (!left));

            if (node == root) {
                struct trie_node *next = node->next;
                if(next != NULL){
                    lock(next, "_insert", "next", __LINE__, locksHeld);
                }
                new_node->next = next;
                node->next = NULL;
                root = new_node;
                if(next != NULL){
                    unlock(next, "_insert", "next", __LINE__, locksHeld);
                }
            } else if (parent) {
                lock(parent, "_insert", "parent",__LINE__, locksHeld);
                assert(parent->children == node);
                new_node->next = NULL;
                parent->children = new_node;
                unlock(parent, "_insert", "parent", __LINE__, locksHeld);
            } else if (left) {
                struct trie_node *next = node->next;
                if(node->next != NULL){
                    unlock(node->next, "_insert", "node.next", __LINE__, locksHeld);
                }
                new_node->next = node->next;
                node->next = NULL;
                left->next = new_node;
                if(next != NULL && *locksHeld > 0){
                    unlock(next, "_insert", "next", __LINE__, locksHeld);
                }
            } else if ((!parent) && (!left)) {
                root = new_node;
            }
            int result = _insert(string, i, ip4_address, node, new_node, NULL, locksHeld);
            return result;
        } else if (cmp < 0) {
            if (node->next == NULL) {
                // Insert here
                struct trie_node *new_node = new_leaf(string, strlen, ip4_address);
                node->next = new_node;
                unlock(node, "_insert", "node", __LINE__, locksHeld);
                return 1;
            } else {
                // No, recur right (the node's key is "greater" than  the search key)
                int result;
                result = _insert(string, strlen, ip4_address, node->next, NULL, node, locksHeld);
               //assert(node != NULL);
                    //unlock(node, "_insert", "node", __LINE__, locksHeld);
                return result;
            }
        } else {
            // Insert here
            struct trie_node *new_node = new_leaf(string, strlen, ip4_address);
            assert(root != NULL);
            if(parent != root && parent != NULL){
                unlock(parent, "_insert", "parent", __LINE__, locksHeld);
            }
            new_node->next = node;
            if (node == root)
                root = new_node;
            else if (parent && parent->children == node)
                parent->children = new_node;
            assert(root != NULL);
            unlock(root, "_insert", "root", __LINE__, locksHeld);
            if(parent != root && parent != NULL){
                unlock(parent, "_insert", "parent", __LINE__, locksHeld);
            }
        }
        if(*locksHeld > 0){
            assert(node != NULL);
            unlock(node, "_insert", "node", __LINE__, locksHeld);
        }
        //unlock(node, "_insert", "node", __LINE__, locksHeld);
        return 1;
    }
}

int insert(const char *string, size_t strlen, int32_t ip4_address) {
    int locksHeld = 0;
    int result;
    struct trie_node *toor = root; //holding root for laeter unlocking
    // Skip strings of length 0
    if (strlen == 0)
        return 0;

    printf("########INSERTING: %s\n", string);

    PRINT("insert","locking rootMutex")
    locksHeld++;
    pthread_mutex_lock(&rootMutex);
    PRINT("insert","locked rootMutex")    

    /* Edge case: root is null */
    if (root == NULL) {
        printf("Creating root\n");
        root = new_leaf(string, strlen, ip4_address);
        PRINT("insert","unlocking rootMutex")
        locksHeld--;
        pthread_mutex_unlock(&rootMutex);
        PRINT("insert","unlocked rootMutex")
        return 1;
    }
    
    PRINT("insert","unlocking rootMutex")
    pthread_mutex_unlock(&rootMutex);
    locksHeld--;
    PRINT("insert","unlocked rootMutex")

    lock(root, "insert", "root", __LINE__, &locksHeld);

    printf("Creating child\n");
    result = _insert(string, strlen, ip4_address, root, NULL, NULL, &locksHeld);
    assert(toor != NULL);
    printf("got before toor\n");
    if(toor != NULL) unlock(toor, "insert", "toor", __LINE__, &locksHeld);
    assert(locksHeld == 0);
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
    int keylen, cmp;

    // First things first, check if we are NULL 
    if (node == NULL) return NULL;

    assert(node->strlen < 64);

    // See if this key is a substring of the string passed in
    cmp = compare_keys(node->key, node->strlen, string, strlen, &keylen);
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
                    assert(node->children == found);
                    node->children = found->next;
                    free(found);
                }

                /* Delete the root node if we empty the tree */
                if (node == root && node->children == NULL && node->ip4_address == 0) {
                    root = node->next;
                    free(node);
                }

                return node; /* Recursively delete needless interior nodes */
            } else
                return NULL;
        } else {
            assert(strlen == keylen);

            /* We found it! Clear the ip4 address and return. */
            if (node->ip4_address) {
                node->ip4_address = 0;

                /* Delete the root node if we empty the tree */
                if (node == root && node->children == NULL && node->ip4_address == 0) {
                    root = node->next;
                    free(node);
                    return (struct trie_node *) 0x100100; /* XXX: Don't use this pointer for anything except 
             * comparison with NULL, since the memory is freed.
             * Return a "poison" pointer that will probably 
             * segfault if used.
             */
                }
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
            if (found->children == NULL && found->ip4_address == 0) {
                assert(node->next == found);
                node->next = found->next;
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
    // Skip strings of length 0
    if (strlen == 0)
        return 0;

    return (NULL != _delete(root, string, strlen));
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
    printf("Root is at %p\n", root);
    /* Do a simple depth-first search */
    if (root)
        _print(root);
}
