#include "main.h"

void bitmap_init(VFS **vfs, int32_t count) {
    (*vfs) -> bitmap = calloc(1, sizeof(BITMAP));
    (*vfs) -> bitmap -> data = (int8_t*) malloc((*vfs) -> superblock -> cluster_count);
    (*vfs) -> bitmap -> length = count;
    for (int i = 0; i < (*vfs) -> bitmap -> length; i++) {
        (*vfs) -> bitmap -> data[i] = 0;
    }
}

int bitmap_contains_free_cluster(BITMAP *bitmap) {
    for (int i = 0; i < bitmap -> length; i++) {
        if (bitmap -> data[i] == 0) {
            return 0;
        }
    }
    return 1;
}

int used_clusters(BITMAP *bitmap) {
    int clusters_used = 0;
    int i;
    for (i = 0; i < bitmap -> length; i++) {
        if (bitmap -> data[i] == 1) clusters_used++;
    }

    return clusters_used;
}

int check_free_clusters (BITMAP **bitmap, int count) {
    int free_clusters = 0;
    for (int i = 0; i < (*bitmap) -> length; i++) {
        if ((*bitmap) -> data[i] == 0) {
            free_clusters++;

            if (free_clusters == count) {
                return 1;
            }
        }
    }
    return 0;
}

int32_t get_one_free_cluster(BITMAP **bitmap) {
    for (int i = 0; i < (*bitmap) -> length; i++) {
        if ((*bitmap) -> data[i] == 0) {
            return i;
        }
    }
    return -1;
}

int32_t set_one_bitmap_on_index (BITMAP **bitmap, int index) {
    if (index > (*bitmap) -> length) {
        return -1;
    }
    (*bitmap) -> data[index] = 1;
    return 1;
}

int32_t set_zero_bitmap_on_index (BITMAP **bitmap, int index) {
    if (index > (*bitmap) -> length) {
        return -1;
    }
    (*bitmap) -> data[index] = 0;
    return 1;
}

void bitmap_info(BITMAP *bitmap) {
    int clusters_used = used_clusters(bitmap);

    printf("+-----------------------------------+\n");
    printf("               [BITMAP]\n");
    printf("  CLUSTER COUNT: %d\n", bitmap -> length);
    printf("  CLUSTERS: [%d - not used] [%d - used]\n", bitmap -> length - clusters_used, clusters_used);
}

void fwrite_bitmap(VFS **vfs) {
    FILE *file;
    file = fopen((*vfs) -> filename, "r+b");

    fseek(file, (*vfs) -> superblock -> bitmap_start_address, SEEK_SET);
    fwrite((*vfs) -> bitmap -> data, sizeof(unsigned char), (*vfs) -> bitmap -> length, file);
    fflush(file);
}

void fread_bitmap(VFS **vfs, FILE *file) {
    (*vfs) -> bitmap = calloc(1, sizeof(BITMAP));
    (*vfs) -> bitmap -> data = (int8_t *)malloc((*vfs) -> superblock -> cluster_count);
    (*vfs) -> bitmap -> length = (*vfs) -> superblock -> cluster_count;
    for (int i = 0; i < (*vfs) -> bitmap -> length; i++) {
        (*vfs) -> bitmap -> data[i] = 0;
    }
    fseek(file, (*vfs) -> superblock -> bitmap_start_address, SEEK_SET);
    fread((*vfs) -> bitmap -> data, sizeof(char unsigned), (*vfs) -> superblock -> cluster_count, file);
}

int32_t *get_free_clusters(VFS **vfs, int count) {
    int i, j = 0;
    int32_t *blocks = (int32_t *)malloc(sizeof(int32_t) * count);

    for (i = 1; i < (*vfs) -> bitmap -> length; i++) {
        if ((*vfs) -> bitmap -> data[i] == 0) {
            blocks[j] = i;
            j++;
            if (j == count) {
                // Set blocks full
                for (int k = 0; k < count; ++k) {
                    (*vfs) -> bitmap -> data[blocks[k]] = 1;
                }
                return blocks;
            }
        }
    }
    free(blocks);
    return NULL;
}