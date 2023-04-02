//
// This file defines the function signatures necessary for creating the three
// cache systems and defines the prefetcher struct.
//

#include "prefetchers.h"

// Null Prefetcher
// ============================================================================
uint32_t null_handle_mem_access(struct prefetcher *prefetcher, struct cache_system *cache_system,
                                uint32_t address, bool is_miss)
{
    return 0; // No lines prefetched
}

void null_cleanup(struct prefetcher *prefetcher) {}

struct prefetcher *null_prefetcher_new()
{
    struct prefetcher *null_prefetcher = calloc(1, sizeof(struct prefetcher));
    null_prefetcher->handle_mem_access = &null_handle_mem_access;
    null_prefetcher->cleanup = &null_cleanup;
    return null_prefetcher;
}

// Sequential Prefetcher
// ============================================================================
// TODO feel free to create additional structs/enums as necessary

uint32_t sequential_handle_mem_access(struct prefetcher *prefetcher,
                                      struct cache_system *cache_system, uint32_t address,
                                      bool is_miss)
{
    uint32_t count_fetched = 0;
    uint32_t amount = *(uint32_t*)(prefetcher->data);

    for(uint32_t i = 1; i <= amount; i++) {
        cache_system_mem_access(cache_system, address + (cache_system->line_size * i), 'w', true);
        // printf("Prefetched %x with base %x\n", address + (cache_system->line_size * i), address);
        count_fetched++;
    }

    // TODO: Return the number of lines that were prefetched.
    // printf("Got %d extra lines in the cache\n", count_fetched);
    return count_fetched;
}

void sequential_cleanup(struct prefetcher *prefetcher)
{
    // TODO cleanup any additional memory that you allocated in the
    // sequential_prefetcher_new function.'
    free(prefetcher->data);
}

struct prefetcher *sequential_prefetcher_new(uint32_t prefetch_amount)
{
    struct prefetcher *sequential_prefetcher = calloc(1, sizeof(struct prefetcher));
    sequential_prefetcher->handle_mem_access = &sequential_handle_mem_access;
    sequential_prefetcher->cleanup = &sequential_cleanup;

    // TODO allocate any additional memory needed to store metadata here and
    // assign to sequential_prefetcher->data.
    sequential_prefetcher->data = malloc(sizeof(uint32_t));
    (*(uint32_t*)sequential_prefetcher->data) = prefetch_amount;

    return sequential_prefetcher;
}

// Adjacent Prefetcher
// ============================================================================
uint32_t adjacent_handle_mem_access(struct prefetcher *prefetcher,
                                    struct cache_system *cache_system, uint32_t address,
                                    bool is_miss)
{
    // TODO perform the necessary prefetches for the adjacent strategy.
    cache_system_mem_access(cache_system, address + cache_system->line_size, 'r', true);
    // TODO: Return the number of lines that were prefetched.
    return 1;
}

void adjacent_cleanup(struct prefetcher *prefetcher)
{
    // TODO cleanup any additional memory that you allocated in the
    // adjacent_prefetcher_new function.
}

struct prefetcher *adjacent_prefetcher_new()
{
    struct prefetcher *adjacent_prefetcher = calloc(1, sizeof(struct prefetcher));
    adjacent_prefetcher->handle_mem_access = &adjacent_handle_mem_access;
    adjacent_prefetcher->cleanup = &adjacent_cleanup;

    // TODO allocate any additional memory needed to store metadata here and
    // assign to adjacent_prefetcher->data.

    return adjacent_prefetcher;
}

// Custom Prefetcher
// ============================================================================
uint32_t custom_handle_mem_access(struct prefetcher *prefetcher, struct cache_system *cache_system,
                                  uint32_t address, bool is_miss)
{
    uint32_t prefetched = 0;

    // int32_t prefetch_amount = *(int32_t*)(prefetcher->data);
    int32_t prev_address = *(int32_t*)(prefetcher->data + (1 * sizeof(int32_t)));
    int32_t stride = *(int32_t*)(prefetcher->data + (2 * sizeof(int32_t)));
    int32_t active = *(int32_t*)(prefetcher->data + (3 * sizeof(int32_t)));

    printf("Prev: %d, Stride: %d, Active %d\n", prev_address, stride, active);
    // TODO perform the necessary prefetches for your custom strategy.
    if (is_miss) {
        // if active is false
        if (active == 0) {
            //  we need to determine the difference between current and prev
            int32_t difference = address - prev_address;

            // if it matches the stored stride, set active to true and prefetch a stride away
            if (stride == difference) {
                printf("Active set to true\n");
                *(int32_t*)(prefetcher->data + (3 * sizeof(int32_t))) = 1;
                cache_system_mem_access(cache_system, address + stride, 'w', true);
                prefetched += 1;
            }

            // else, prefetch 1 away and update the stride value to the difference
            else {
                printf("Active kept at false\n");
                // cache_system_mem_access(cache_system, address + cache_system->line_size, 'w', true);
                prefetched += sequential_handle_mem_access(prefetcher, cache_system, address, is_miss);
                *(int32_t*)(prefetcher->data + (2 * sizeof(int32_t))) = difference;
            }
        }

        // if active if true
        else {
            printf("Active set to false\n");
            // set active to false, we had a miss
            *(int32_t*)(prefetcher->data + (3 * sizeof(int32_t))) = 0;
            prefetched += sequential_handle_mem_access(prefetcher, cache_system, address, is_miss);
        }
    } else {
        // stride is active, so just prefetch the stride
        if (active == 1) {
            printf("Strided prefetch\n");
            cache_system_mem_access(cache_system, address + stride, 'w', true);
            prefetched += 1;
        } else {
            printf("1 Lookahead\n");
            prefetched += sequential_handle_mem_access(prefetcher, cache_system, address, is_miss);
            int32_t difference = address - prev_address;
            *(int32_t*)(prefetcher->data + (2 * sizeof(int32_t))) = difference;
        }
    }

    // update prev address
    *(int32_t*)(prefetcher->data + (1 * sizeof(int32_t))) = address;

    return prefetched;
    // if (is_miss) {
    //         // cache_system_mem_access(cache_system, address + cache_system->line_size, 'r', true);
    //         int32_t diff = address - *(uint32_t*)(prefetcher->data);
    //         uint32_t current_address = address;

    //         // uint32_t target_address = address + stride;
    //         if ((diff > 0) == true) {
    //             while ((current_address < address + diff) == true) {
    //                 printf("Diff: %d\n", diff);
    //                 printf("boolean: %d\n", (diff > 0));
    //                 //printf("", current_address, );
    //                 cache_system_mem_access(cache_system, current_address, 'r', true);
    //                 current_address = current_address + cache_system->line_size;
    //                 prefetched += 1;
    //             }
    //         }

    //         // cache_system_mem_access(cache_system, address + stride, 'r', true);

    //         //reassign the previous address
    //         *(uint32_t*)(prefetcher->data) = address;
    //     }
        
    //     // TODO: Return the number of lines that were prefetched.
    //     return prefetched;
}

void custom_cleanup(struct prefetcher *prefetcher)
{
    // TODO cleanup any additional memory that you allocated in the
    // custom_prefetcher_new function.
    free(prefetcher->data);
}

struct prefetcher *custom_prefetcher_new(uint32_t prefetch_amount)
{
    struct prefetcher *custom_prefetcher = calloc(1, sizeof(struct prefetcher));
    custom_prefetcher->handle_mem_access = &custom_handle_mem_access;
    custom_prefetcher->cleanup = &custom_cleanup;

    // TODO allocate any additional memory needed to store metadata here and
    // assign to custom_prefetcher->data.

    // four parameters are stored here:
    // 1. prefetch amount if not striding
    // 1. the previous address
    // 2. the stride length (tells us how far to the next address)
    // 3. whether or not the stride is active
    //      - on active, we prefetch a stride away
    //      - on inactive, we simply to an adjacent prefetch
    custom_prefetcher->data = calloc(4, sizeof(int32_t));
    (*(int32_t*)custom_prefetcher->data) = (int32_t)prefetch_amount;


    return custom_prefetcher;
}
