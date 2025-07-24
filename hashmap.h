#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Define a macro for calculating the hash index
#define HASH_INDEX(key, size) (hash(key) % (unsigned long)(size))

// Structure to represent a key-value pair in the hashmap.
// It includes a pointer to the next pair to handle collisions using separate chaining.
typedef struct pair
{
    char *key;
    int value;
    struct pair *next; // Pointer to the next pair in case of a collision (linked list)
} pair;

// Structure to represent the hashmap itself.
// It contains the number of buckets and an array of pointers to pairs (the buckets).
typedef struct hashmap
{
    int size;          // Number of buckets in the hashmap
    pair **buckets;    // Array of pointers to pairs (each bucket is a linked list)
} hashmap;

// Enum for function return status
typedef enum HashMapStatus{
    HM_SUCCESS = 0,
    HM_ERR_MALLOC_FAILED,
    HM_ERR_KEY_NOT_FOUND,
    HM_ERR_INVALID_ARG,
} HashMapStatus;

// Function declarations
unsigned long hash(const char* string);                    // Hashes a string to an unsigned long
hashmap* c_hashmap(int size);                              // Creates and initializes a new hashmap
HashMapStatus put(hashmap* map, const char *key, int value); // Inserts or updates a key-value pair
HashMapStatus get(const hashmap* map, const char *key, int *value); // Retrieves the value associated with a key
HashMapStatus delete_key(hashmap* map, const char *key);   // Deletes a key-value pair
void d_hashmap(hashmap* map);                             // Frees all memory associated with the hashmap

#endif