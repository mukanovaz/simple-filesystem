#include "main.h"

void bitmap_init(BITMAP **bitmap, int32_t count) {
    (*bitmap) = calloc(1, sizeof(BITMAP));
    (*bitmap) -> length = count;
    (*bitmap) -> data = calloc((*bitmap) -> length, sizeof(unsigned char));
    memset((*bitmap) -> data, 0, (*bitmap) -> length);
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