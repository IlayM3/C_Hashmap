#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

// A macro for calculating the hash index
#define HASH_INDEX(key, size) (hash(key) % (unsigned long)(size))
// A macro for calculating the load factor
#define LOAD_FACTOR(map) ((float) map -> count / map -> size)
// A macro for our max factor (might lower it later)
#define MAX_FACTOR 0.75

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
    int count;         // Number of key-value pairs
} hashmap;

// Enum for function return status
typedef enum HashMapStatus{
    HM_SUCCESS = 0,
    HM_ERR_MALLOC_FAILED,
    HM_ERR_KEY_NOT_FOUND,
    HM_ERR_INVALID_ARG,
    HM_ERR_REHASHING_FAILED,
    HM_ERR_CLEAR_FAILED,
    HM_ERR_SIZE_LIMIT,
} HashMapStatus;

// Function declarations
unsigned long hash(const char* string);                    // Hashes a string to an unsigned long
hashmap* c_hashmap(int size);                              // Creates and initializes a new hashmap
HashMapStatus put(hashmap* map, const char *key, int value); // Inserts or updates a key-value pair
HashMapStatus get(const hashmap* map, const char *key, int *value); // Retrieves the value associated with a key
HashMapStatus delete_key(hashmap* map, const char *key);   // Deletes a key-value pair
HashMapStatus resize(hashmap* map);                        // Dynamic resizing
// TODO: 
bool contains_key(const hashmap* map, const char *key);   // Checks if a key exists in the hashmap
HashMapStatus clear(hashmap* map);                         // Clears all key-value pairs from the hashmap
void p_hashmap(const hashmap* map);                   // Prints the contents of the hashmap
void d_hashmap(hashmap* map);                             // Frees all memory associated with the hashmap
