
#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



int main(int argc, char *argv[]) {
    
    int curr_num;
    int source = 0;              // used to know source to recv. from or that it should send messages
    int num_of_primes = 0;
    int potential_prime = 1;     // set to 0 when number evenly divides
    int new_prime = -1;         // used for receiving new prime numbers from other processes
    int msg_index = 0;
    
    const int limit = 1000;
    int primes[limit];
    
    for (int i = 0; i < limit; i++) {     //  initialize array to -1
        primes[i] = -1;
    }
    
    MPI_Status stat;
    
    
    // Initialize the MPI environment
    MPI_Init(NULL, NULL);
    
    
    // Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    
    
    
    // Get the rank of the process
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    
    curr_num = 2 + world_rank;
    
    
    
    while (num_of_primes < limit) {
        
        
        if (world_rank == source) {                                     // SENDER
            if (potential_prime) {                // check if prime
                // num. is prime
                primes[num_of_primes] = curr_num;
                printf("Prime number #%d is: %d\r\n", (num_of_primes+1), curr_num);
                for (int i = 0; i < world_size; i++) {
                    if (i != source) {                // send to everyone but yourself
                        MPI_Send(&curr_num, 1, MPI_INT, i, 0, MPI_COMM_WORLD);  // give prime
                        //printf("%d sent by %d to %d\r\n", curr_num, world_rank, i);
                    }
                }
                num_of_primes++;                 // increment the num of primes
                
            } else {
                int not_prime = -1;               // the "error code" for a number not being prime
                for (int i = 0; i < world_size; i++) {
                    if (i != source) {                // send to everyone but yourself
                        MPI_Send(&not_prime, 1, MPI_INT, i, 0, MPI_COMM_WORLD);  // no prime; unblock others
                        //printf("%d sent for %d by %d to %d\r\npot. prime: %d\r\n", not_prime, curr_num, world_rank, i, potential_prime);
                    }
                }
                msg_index++;                        // message sent out
            }
            source = (source + 1) % 10;            // go 0-9 and then wrap back around
            potential_prime = 1;          // reset
            curr_num += world_size; // give this process a new number
            
        } else {                                                        // RECIEVER
            int new_prime;          // -1 if not prime
            int testing_prime = primes[0];
            for (int i = 1; testing_prime != -1 && i < limit; i++) {
                if (curr_num % testing_prime == 0) {
                    potential_prime = 0;          // not prime if the curr_num evenly divides
                    //printf("potential prime %d divides evenly: %d\r\nRuled out by: %d\r\n", curr_num, potential_prime, world_rank);
                }
                testing_prime = primes[i];
                // potential edge case
            }
            MPI_Recv(&new_prime, 1, MPI_INT, source, 0, MPI_COMM_WORLD, &stat); // potentially receive a prime
            msg_index++;            // message received
            //printf("%d recieved by %d\r\nSource: %d\r\n", new_prime, world_rank, stat.MPI_SOURCE);
            if (new_prime != -1) {
                primes[num_of_primes] = new_prime;
                num_of_primes++;                 // increment the num of primes
                if (curr_num % new_prime == 0) {
                    potential_prime = 0;          // not prime if the curr_num evenly divides
                    //printf("potential prime %d divides evenly: %d\r\nRuled out by: %d\r\n", curr_num, potential_prime, world_rank);
                }
            }
            // potential prime is now correctly updated
            source = (source + 1) % 10;           // go 0-9 and then wrap back around
        }
    }
    
    
    // Finalize the MPI environment.
    MPI_Finalize();
    return 1;
}
