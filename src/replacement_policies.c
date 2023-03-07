//
// This file contains all of the implementations of the replacement_policy
// constructors from the replacement_policies.h file.
//
// It also contains stubs of all of the functions that are added to each
// replacement_policy struct at construction time.
//
// ============================================================================
// NOTE: It is recommended that you read the comments in the
// replacement_policies.h file for further context on what each function is
// for.
// ============================================================================
//

#include "replacement_policies.h"

// LRU Replacement Policy
// ============================================================================

void lru_cache_access(struct replacement_policy *replacement_policy,
                      struct cache_system *cache_system, uint32_t set_idx, uint32_t tag)
{
    // DONE update the LRU replacement policy state given a new memory access
    uint32_t set_start = set_idx * cache_system->associativity;
    void *set_values = (replacement_policy->data + (set_idx * cache_system->associativity * sizeof(uint32_t)));

    // find the current highest value
    uint32_t highest_value = 0;
    //printf("Timestamps: ");
    for (uint32_t i = 0; i < cache_system->associativity; i++) {
        //printf("%d ", *(uint32_t*)(set_values + (i * sizeof(uint32_t))));
        if (*(uint32_t*)(set_values + (i * sizeof(uint32_t))) > highest_value) highest_value = *(uint32_t*)(set_values + (i * sizeof(uint32_t)));
    }
    //printf("\nHighest_value: %d\n", highest_value);

    for (uint32_t i = 0; i < cache_system->associativity; i++) {
        //printf("current: 0x%x needed: 0x%x\n", cache_system->cache_lines[set_start + i].tag, tag);
        if (cache_system->cache_lines[set_start + i].tag == tag) {
            //printf("Index %d set to %d\n", i, highest_value + 1);
            (*(uint32_t*)(set_values + (i * sizeof(uint32_t)))) = highest_value + 1;
            break;
        }
    }
}

uint32_t lru_eviction_index(struct replacement_policy *replacement_policy,
                            struct cache_system *cache_system, uint32_t set_idx)
{
    // DONE return the index within the set that should be evicted.
    void *set_values = (replacement_policy->data + (set_idx * cache_system->associativity * sizeof(uint32_t)));

    //printf("Set: %d ", set_idx);
    uint32_t index = 0;
    for (uint32_t i = 0; i < cache_system->associativity; i++) {
        //printf("%d ", (*(uint32_t*)(set_values + (i * sizeof(uint32_t)))));
        if ((*(uint32_t*)(set_values + (i * sizeof(uint32_t)))) < (*(uint32_t*)(set_values + (index * sizeof(uint32_t))))) index = i;
    }
    //printf("\n");

    return index;
}

void lru_replacement_policy_cleanup(struct replacement_policy *replacement_policy)
{
    // DONE cleanup any additional memory that you allocated in the
    // lru_replacement_policy_new function.
    free(replacement_policy->data);
}

struct replacement_policy *lru_replacement_policy_new(uint32_t sets, uint32_t associativity)
{
    struct replacement_policy *lru_rp = calloc(1, sizeof(struct replacement_policy));
    lru_rp->cache_access = &lru_cache_access;
    lru_rp->eviction_index = &lru_eviction_index;
    lru_rp->cleanup = &lru_replacement_policy_cleanup;

    // DONE allocate any additional memory to store metadata here and assign to
    // lru_rp->data.

    // one int for each line in the cache
    // the smallest number among the lines in  a set is least recently used
    lru_rp->data = calloc(sets * associativity, sizeof(uint32_t));

    return lru_rp;
}

// RAND Replacement Policy
// ============================================================================

void rand_cache_access(struct replacement_policy *replacement_policy,
                       struct cache_system *cache_system, uint32_t set_idx, uint32_t tag)
{
    // DONE update the RAND replacement policy state given a new memory access
}

uint32_t rand_eviction_index(struct replacement_policy *replacement_policy,
                             struct cache_system *cache_system, uint32_t set_idx)
{
    // DONE return the index within the set that should be evicted.
    printf("assosciativity: %d\n", cache_system->associativity);
    uint32_t index = rand() % (cache_system->associativity);
    printf("evicting: %d", index);
    return index;
}

void rand_replacement_policy_cleanup(struct replacement_policy *replacement_policy)
{
    // DONE cleanup any additional memory that you allocated in the
    // rand_replacement_policy_new function.
}

struct replacement_policy *rand_replacement_policy_new(uint32_t sets, uint32_t associativity)
{
    // Seed randomness
    srand(time(NULL));

    struct replacement_policy *rand_rp = malloc(sizeof(struct replacement_policy));
    rand_rp->cache_access = &rand_cache_access;
    rand_rp->eviction_index = &rand_eviction_index;
    rand_rp->cleanup = &rand_replacement_policy_cleanup;

    // DONE allocate any additional memory to store metadata here and assign to
    // rand_rp->data.

    return rand_rp;
}

// LRU_PREFER_CLEAN Replacement Policy
// ============================================================================
void lru_prefer_clean_cache_access(struct replacement_policy *replacement_policy,
                                   struct cache_system *cache_system, uint32_t set_idx,
                                   uint32_t tag)
{
    // DONE update the LRU_PREFER_CLEAN replacement policy state given a new
    // memory access
    // NOTE: you may be able to share code with the LRU policy
    lru_cache_access(replacement_policy, cache_system, set_idx, tag);
}

uint32_t lru_prefer_clean_eviction_index(struct replacement_policy *replacement_policy,
                                         struct cache_system *cache_system, uint32_t set_idx)
{
    // DONE return the index within the set that should be evicted.
    uint32_t set_start = set_idx * cache_system->associativity;
    void *set_values = (replacement_policy->data + (set_idx * cache_system->associativity * sizeof(uint32_t)));

    printf("Set: %d ", set_idx);
    uint32_t index = -1;
    bool got_clean = false;
    for (uint32_t i = 0; i < cache_system->associativity; i++) {
        printf("%d ", (*(uint32_t*)(set_values + (i * sizeof(uint32_t)))));
        if (cache_system->cache_lines[set_start + i].status != MODIFIED) {
            printf("\n");
            if ((index == -1 || (*(uint32_t*)(set_values + (i * sizeof(uint32_t)))) < (*(uint32_t*)(set_values + (index * sizeof(uint32_t)))))) {
                index = i;
                got_clean = true;
            }
        }
        else {
            printf("DIRTY\n");
        }
    }

    if (got_clean == false) {
        index = lru_eviction_index(replacement_policy, cache_system, set_idx);
        printf("DIRTY EVICT\n");
    }

    return index;
}

void lru_prefer_clean_replacement_policy_cleanup(struct replacement_policy *replacement_policy)
{
    // DONE cleanup any additional memory that you allocated in the
    // lru_prefer_clean_replacement_policy_new function.
    free(replacement_policy->data);
}

struct replacement_policy *lru_prefer_clean_replacement_policy_new(uint32_t sets,
                                                                   uint32_t associativity)
{
    struct replacement_policy *lru_prefer_clean_rp = malloc(sizeof(struct replacement_policy));
    lru_prefer_clean_rp->cache_access = &lru_prefer_clean_cache_access;
    lru_prefer_clean_rp->eviction_index = &lru_prefer_clean_eviction_index;
    lru_prefer_clean_rp->cleanup = &lru_prefer_clean_replacement_policy_cleanup;

    // DONE allocate any additional memory to store metadata here and assign to
    // lru_prefer_clean_rp->data.
    lru_prefer_clean_rp->data = calloc(sets * associativity, sizeof(uint32_t));

    return lru_prefer_clean_rp;
}

