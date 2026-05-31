#ifndef NKFS_H
#define NKFS_H

#include <stdint.h>
#include <stddef.h>

#define NKFS_FILE 0
#define NKFS_DIR  1

typedef struct nkfs_node {
    char name[32];
    uint8_t type;
    uint32_t size;          
    uint8_t* data;          
    
    struct nkfs_node* parent;
    struct nkfs_node* first_child;
    struct nkfs_node* next_sibling;
} nkfs_node_t;

void nkfs_init(void);
nkfs_node_t* nkfs_get_root(void);
nkfs_node_t* nkfs_find_child(nkfs_node_t* parent, const char* name);
nkfs_node_t* nkfs_mkdir(nkfs_node_t* parent, const char* name);
nkfs_node_t* nkfs_create_file(nkfs_node_t* parent, const char* name);
int nkfs_write_file(nkfs_node_t* file, const char* data, uint32_t size);

#endif
