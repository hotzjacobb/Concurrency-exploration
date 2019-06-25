#include <uv.h>
#include <stdio.h>
#include <stdlib.h>


struct Node
{
    int data;
    struct Node* next;
};

struct LinkedList
{
    struct Node* head;
    struct Node* tail;
};

int limit;
int count;
struct LinkedList* primes_discovered;

void compute_next(uv_idle_t* handle) {
    count++;                                // one more prime found
    int potential_prime = primes_discovered->tail->data;
    int prime_not_found = 1;
    do {                // look for the next prime, break once found
        potential_prime++;
        struct Node* divisor = primes_discovered->head;
        int could_be_prime = 1;
        while (could_be_prime) {           // iterate through primes discovered
            could_be_prime = potential_prime % divisor->data;       // mod potential prime by discovered prime
            divisor = divisor->next;            // test on next known prime
            //printf("divisor: %p\r\n", divisor);
            if (could_be_prime && divisor == NULL) {         // check if hasn't divided evenly and "next prime" is null
                // potential_prime is a prime
                printf("prime #%d found: %d\r\n", count, potential_prime);
                prime_not_found = 0;
                primes_discovered->tail->next = (struct Node*) malloc(sizeof(struct Node));
                primes_discovered->tail->next->data = potential_prime;
                primes_discovered->tail->next->next = NULL;
                primes_discovered->tail = primes_discovered->tail->next;
                if (count == limit) {
                    uv_idle_stop(handle);              // setting idle to inactive will cause loop to die as it is the only handle
                }
                break;                                 // prime found, break
            }
        }
    } while (prime_not_found);                            
}


int main(int argc, char **argv) {
    printf("Welcome to an event driven sieve\r\n");
    
    if (argc != 2) // wrong args passed
    {
        printf("Please specify a number to count to as an argument\r\n");
        return 1;
    }
    
    count = 0;
    limit = atoi(argv[1]);
    
    if (limit < 1) // arg is less than one
    {
        printf("Please make the limit at least 1\r\n");
        return 1;
    }
    
    
    primes_discovered = (struct LinkedList*) malloc(sizeof(struct LinkedList));  // initialize linked list
    primes_discovered->head = (struct Node*) malloc(sizeof(struct Node));
    primes_discovered->head->next = NULL;
    primes_discovered->head->data = 2;                    // make the first prime two
    primes_discovered->tail = primes_discovered->head;
    
    
    uv_loop_t *loop = malloc(sizeof(uv_loop_t));          // alloc. uv event loop
    uv_loop_init(loop);
    
    uv_idle_t idle_next;                                  // use idle handle to calc. primes each it. of loop
    uv_idle_init(loop, &idle_next);
    uv_idle_start(&idle_next, compute_next);
    
    uv_run(loop, UV_RUN_DEFAULT);                      // start loop
    uv_loop_close(loop);
    free(loop);                                        // free even though we're closing anyway
    struct Node* node_to_delete = primes_discovered->head;
    while (!node_to_delete) {
        struct Node* temp_ptr = node_to_delete;
        node_to_delete = node_to_delete->next;
        free(temp_ptr);
    }
    free(primes_discovered);
    return 0;
}

