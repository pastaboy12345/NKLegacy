#include "nkfs.h"
#include "../../kernel/kprintf.h"
#include <nk/string.h>

#define MAX_NODES 256
#define DATA_POOL_SIZE (512 * 1024)

static nkfs_node_t node_pool[MAX_NODES];
static uint32_t node_count = 0;

static uint8_t data_pool[DATA_POOL_SIZE];
static uint32_t data_pool_used = 0;

static nkfs_node_t* root_node = (void*)0;

void nkfs_init(void) {
    memset(node_pool, 0, sizeof(node_pool));
    memset(data_pool, 0, sizeof(data_pool));
    node_count = 0;
    data_pool_used = 0;

    // Create Root Node
    root_node = &node_pool[node_count++];
    strcpy(root_node->name, "C:");
    root_node->type = NKFS_DIR;
    root_node->parent = (void*)0;
    
    kprintf("NKFS: RAM filesystem initialized (Capacity: %d KB).\n", DATA_POOL_SIZE / 1024);
}

nkfs_node_t* nkfs_get_root(void) {
    return root_node;
}

nkfs_node_t* nkfs_find_child(nkfs_node_t* parent, const char* name) {
    if (!parent || parent->type != NKFS_DIR) return (void*)0;
    nkfs_node_t* child = parent->first_child;
    while (child) {
        if (strcmp(child->name, name) == 0) return child;
        child = child->next_sibling;
    }
    return (void*)0;
}

nkfs_node_t* nkfs_mkdir(nkfs_node_t* parent, const char* name) {
    if (!parent || parent->type != NKFS_DIR || node_count >= MAX_NODES) return (void*)0;
    if (nkfs_find_child(parent, name)) return (void*)0;

    nkfs_node_t* child = &node_pool[node_count++];
    strncpy(child->name, name, 31);
    child->type = NKFS_DIR;
    child->parent = parent;
    
    child->next_sibling = parent->first_child;
    parent->first_child = child;
    parent->size++;

    return child;
}

nkfs_node_t* nkfs_create_file(nkfs_node_t* parent, const char* name) {
    if (!parent || parent->type != NKFS_DIR || node_count >= MAX_NODES) return (void*)0;
    
    nkfs_node_t* child = nkfs_find_child(parent, name);
    if (!child) {
        child = &node_pool[node_count++];
        strncpy(child->name, name, 31);
        child->type = NKFS_FILE;
        child->parent = parent;
        
        child->next_sibling = parent->first_child;
        parent->first_child = child;
        parent->size++;
    } else if (child->type == NKFS_DIR) {
        return (void*)0;
    }

    child->size = 0;
    child->data = (void*)0;
    
    return child;
}

int nkfs_write_file(nkfs_node_t* file, const char* data, uint32_t size) {
    if (!file || file->type != NKFS_FILE) return -1;
    if (data_pool_used + size > DATA_POOL_SIZE) return -1;

    file->data = &data_pool[data_pool_used];
    memcpy(file->data, data, size);
    file->size = size;
    data_pool_used += size;

    return 0;
}
