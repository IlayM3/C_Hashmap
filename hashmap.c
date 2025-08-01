#pragma GCC optimize("O3")

#include "hashmap.h"

/**
 * @brief Computes a hash value for a given string using the DJB2 algorithm.
 * @param string The input string to hash.
 * @return An unsigned long hash value.
 */
unsigned long hash(const char* string)
{
    unsigned long hash_val = 5381; // DJB2
    int c;

    while ((c = *string++)) {
        // hash * 33 + c
        hash_val = ((hash_val << 5) + hash_val) + c;
    }

    return hash_val;
}

/**
 * @brief Creates and initializes a new hashmap.
 * @param size The desired number of buckets for the hashmap.
 * @return A pointer to the newly created hashmap, or NULL if memory allocation fails.
 */
hashmap* c_hashmap(int size)
{
    // Validate input size
    if (size <= 0) {
        fprintf(stderr, "Error: Hashmap size must be positive.\n");
        return NULL;
    }

    // Allocate memory for the hashmap structure
    hashmap* map = malloc(sizeof(hashmap));
    if (!map) {
        perror("Error: Failed to allocate memory for hashmap");
        return NULL;
    }

    map->size = size;
    map->count = 0; // Initialize count to 0
    // Allocate memory for the array of bucket pointers and initialize them to NULL
    map->buckets = calloc(size, sizeof(pair*)); // calloc initializes memory to zero (NULL for pointers)
    if (!map->buckets) {
        perror("Error: Failed to allocate memory for hashmap buckets");
        free(map); // Free the hashmap structure if bucket allocation fails
        return NULL;
    }

    return map;
}

/**
 * @brief Inserts a new key-value pair into the hashmap, or updates the value if the key already exists.
 * @param map A pointer to the hashmap.
 * @param key The string key.
 * @param value The integer value.
 * @return HashMapStatus indicating success or failure type.
 */
HashMapStatus put(hashmap* map, const char *key, int value)
{
    // Validate inputs
    if (!map || !key) {
        fprintf(stderr, "Error: Invalid hashmap or key provided to put.\n");
        return HM_ERR_INVALID_ARG;
    }

    // Calculate the index for the key using the HASH_INDEX macro
    unsigned long index = HASH_INDEX(key, map->size);

    // Traverse the linked list at the calculated index to check if the key already exists
    pair* current = map->buckets[index];
    while (current) {
        if (strcmp(current->key, key) == 0) {
            // Key found, update its value and return
            current->value = value;
            return HM_SUCCESS;
        }
        current = current->next;
    }

    // If the key does not exist, create a new pair
    pair* new_pair = malloc(sizeof(pair));
    if (!new_pair) {
        perror("Error: Failed to allocate memory for new pair");
        return HM_ERR_MALLOC_FAILED;
    }

    // Allocate memory for the key string and copy it
    new_pair->key = malloc(strlen(key) + 1);
    if (!new_pair->key) {
        perror("Error: Failed to allocate memory for key string");
        free(new_pair);
        return HM_ERR_MALLOC_FAILED;
    }
    strcpy(new_pair->key, key);

    new_pair->value = value;
    // Insert the new pair at the head of the linked list in the bucket
    new_pair->next = map->buckets[index];
    map->buckets[index] = new_pair;
    map->count++; // Increment count when a new pair is added

    // If a new pair was successfully added (map->count was incremented):
    if (LOAD_FACTOR(map) > MAX_FACTOR) {
        HashMapStatus status = resize(map);
        if (status != HM_SUCCESS) {
            fprintf(stderr, "Warning: Hashmap resize failed after put.\n");
            return HM_ERR_REHASHING_FAILED;
        }
    }

    return HM_SUCCESS;
}

/**
 * @brief Retrieves the value associated with a given key from the hashmap.
 * @param map A constant pointer to the hashmap (data won't be modified).
 * @param key The string key to search for.
 * @param value Pointer to store the retrieved value.
 * @return HashMapStatus indicating success or failure type.
 */
HashMapStatus get(const hashmap* map, const char *key, int *value)
{
    // Validate inputs
    if (!map || !key || !value) {
        fprintf(stderr, "Error: Invalid hashmap, key, or value pointer provided to get.\n");
        return HM_ERR_INVALID_ARG;
    }

    // Calculate the index for the key using the HASH_INDEX macro
    unsigned long index = HASH_INDEX(key, map->size);
    pair* current = map->buckets[index];

    // Traverse the linked list at the calculated index
    while (current) {
        if (strcmp(current->key, key) == 0) {
            *value = current->value; // Key found, store its value
            return HM_SUCCESS;
        }
        current = current->next;
    }

    return HM_ERR_KEY_NOT_FOUND;
}

/**
 * @brief Deletes a key-value pair from the hashmap.
 * @param map A pointer to the hashmap.
 * @param key The string key of the pair to delete.
 * @return HashMapStatus indicating success or failure type.
 */
HashMapStatus delete_key(hashmap* map, const char *key)
{
    // Validate inputs
    if (!map || !key) {
        fprintf(stderr, "Error: Invalid hashmap or key provided to delete_key.\n");
        return HM_ERR_INVALID_ARG;
    }

    // Calculate the index for the key using the HASH_INDEX macro
    unsigned long index = HASH_INDEX(key, map->size);
    pair* current = map->buckets[index];
    pair* prev = NULL; // Pointer to the previous pair in the linked list

    // Traverse the linked list to find the key to delete
    while (current) {
        if (strcmp(current->key, key) == 0) {
            // Key found, remove it from the linked list
            if (prev) {
                prev->next = current->next; // Link previous to next, bypassing current
            } else {
                map->buckets[index] = current->next; // If deleting head, update bucket pointer
            }

            // Free the memory for the key string and the pair structure
            free(current->key);
            free(current);
            map->count--; // Decrement count when a pair is deleted
            return HM_SUCCESS;
        }
        prev = current;      // Move prev to current
        current = current->next; // Move current to next
    }

    return HM_ERR_KEY_NOT_FOUND;
}

/**
 * @brief Resizes the hashmap by creating a new, larger array of buckets
 * and re-hashing all existing key-value pairs into the new structure.
 * @param map A pointer to the hashmap to be resized.
 * @return HashMapStatus indicating success or failure type.
 */
HashMapStatus resize(hashmap* map)
{
    // Validate input
    if (!map) {
        fprintf(stderr, "Error: Invalid hashmap provided to resize.\n");
        return HM_ERR_INVALID_ARG;
    }

    // Calculate the new size (double the current size)
    size_t newSize;
    if (__builtin_mul_overflow(map->size, 2, &newSize)) {
        fprintf(stderr, "Error: Cannot resize hashmap - size overflow.\n");
        return HM_ERR_SIZE_LIMIT;
    }

    // Allocate memory for the new array of buckets and initialize to NULL
    pair** newBuckets = calloc(newSize, sizeof(pair*));
    if (!newBuckets) {
        fprintf(stderr, "Error: Failed to allocate memory for new buckets during resize.\n");
        return HM_ERR_MALLOC_FAILED;
    }

    // Store old buckets and old size before modifying the map structure
    pair** oldBuckets = map->buckets;
    int oldSize = map->size;

    // Update map's properties to the new values immediately.
    map->size = newSize;
    map->buckets = newBuckets;
    map->count = 0; 

    // Iterate through each bucket in the old hashmap
    for (int i = 0; i < oldSize; i++) {
        pair* current = oldBuckets[i];
        // Traverse the linked list in the current old bucket
        while (current) {
            pair* temp = current;
            current = current->next; // Move to the next pair in the old list before processing temp

            // Calculate the new index for the current pair's key in the new hashmap
            unsigned long newIndex = HASH_INDEX(temp->key, map->size); 

            // Insert the current pair (temp) at the head of the linked list
            // in the appropriate new bucket. 
            temp->next = map->buckets[newIndex];
            map->buckets[newIndex] = temp;
            map->count++; // Increment count for each re-inserted element
        }
    }

    // Free the memory allocated for the old array of buckets
    free(oldBuckets);

    return HM_SUCCESS;
}

/**
 * @brief Frees all memory allocated for the hashmap.
 * @param map A pointer to the hashmap to be deallocated.
 */
void d_hashmap(hashmap* map)
{
    if (!map) {
        return; // Nothing to free if map is NULL
    }

    // Iterate through each bucket
    for (int i = 0; i < map->size; i++) {
        pair* current = map->buckets[i];
        // Traverse the linked list in the current bucket and free each pair
        while (current) {
            pair* temp = current; // Store current pair to free it
            current = current->next; // Move to the next pair before freeing temp
            free(temp->key);         // Free the dynamically allocated key string
            free(temp);              // Free the pair structure itself
        }
    }

    // Free the array of bucket pointers
    free(map->buckets);
    // Free the hashmap structure itself
    free(map);
}
