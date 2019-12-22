#include "main.h"

void bitmap_init(BITMAP **bitmap, int32_t count) {
    (*bitmap) = calloc(1, sizeof(BITMAP));
    (*bitmap) -> length = count;
    for (int i = 0; i < (*bitmap) -> length; i++) {
        (*bitmap) -> data[i] = 0;
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

struct the_fragment_temp *find_free_cluster(BITMAP **bitmap, int needed_count) {
    struct the_fragment_temp *temp = calloc(1, sizeof(struct the_fragment_temp));
    temp -> start_cluster_ID = -1;
    temp -> count = 0;
    temp -> successful = 0;
    int i;
    int count = 0;
    for (i = 0; i < (*bitmap) -> length; i++) {
        if ((*bitmap) -> data[i] == 0) {
            if (count == 0) temp -> start_cluster_ID = i;
            (*bitmap) -> data[i] = 1;
            count++;
            temp -> count = count;

            if (count == needed_count) {
                temp -> successful = 1;
                return temp;
            }
        }
        else {
            if (count > 0) {
                temp -> successful = 0;
                return temp;
            }
        }
    }
    return temp;
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
    printf("\nBitmap:\n----------------\n");
    printf("Length: %d\n", bitmap -> length);
    printf("Clusters ((%dx) 0 - not used; (%dx) 1 - used):", bitmap -> length - clusters_used, clusters_used);

    int i;
    for (i = 0; i < bitmap -> length; i++) {
        if (i % 8 == 0) printf("\n");
        printf("%d", bitmap -> data[i]);
    }
    printf("\n");
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
    (*vfs) -> bitmap -> length = (*vfs) -> superblock -> cluster_count;
    for (int i = 0; i < (*vfs) -> bitmap -> length; i++) {
        (*vfs) -> bitmap -> data[i] = 0;
    }
    fseek(file, (*vfs) -> superblock -> bitmap_start_address, SEEK_SET);
    fread((*vfs) -> bitmap -> data, sizeof(char unsigned), (*vfs) -> superblock -> cluster_count, file);
}
