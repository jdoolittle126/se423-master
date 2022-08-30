//
// Simple program demonstrating shared memory in POSIX systems.
// Producer - Consumer Battle 
//
//*************************************************************
//*************************************************************
// Declarations
//*************************************************************
#include <signal.h>
#include <stdio.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <pthread.h>
#include <cstdlib>
#include <unistd.h>
#include <iostream>
#include <semaphore.h>

//*************************************************************
// Play with these constants to adjust the simulation
//*************************************************************

// Seconds to run factory simulation
const int SIM_LENGTH = 30;

// Number of widget slots
const int BUFFER_SIZE = 200;

// Max number of producers
const int MAX_PRODUCERS = 5;

// Max number of comsumers
const int MAX_CONSUMERS = 5;

// Max time to sleep 
const int SLEEP_RANGE = 5;

// Terminal character color codes
#define VT_BLACK  	"\033[1;30m"
#define VT_RED    	"\033[7;31m"
#define VT_GREEN  	"\033[0;1;32m"
#define VT_BROWN  	"\033[1;33m"
#define VT_BLUE   	"\033[0;1;34m"
#define VT_MAGENTA  "\033[1;35m"
#define VT_CYAN   	"\033[1;36m"
#define VT_RESET  	"\033[0m"

// Function protoypes
void* producer(void* args);
void* consumer(void* args);
void  printMenu();

// Structure to hold shared data 
typedef struct {
	int next_in;			// index of next widget to go in
	int next_out;			// index of next widget to come out
	int widget_count;		// number of widgets in buffer
	int buffer[BUFFER_SIZE];// widget buffer
} widget_factory_t;

//  a pointer to the shared memory segment 
	widget_factory_t* shared_factory;
    pthread_mutex_t count_lock;
    sem_t empty_slots;
    sem_t full_slots;

//*************************************************************
//*************************************************************
// main function
//*************************************************************
//*************************************************************
int main() {
   
   //*************************************************************
   // Initializations
   //*************************************************************
	// These arrays hold the system thread ID structures of the threads
	pthread_t producer_tid[MAX_PRODUCERS];
	pthread_t consumer_tid[MAX_CONSUMERS];

	// Allocate memory for the Widget Factory
	shared_factory = new widget_factory_t;
	
	// Assume we need more tests
	bool moreTests = true;
	
	// Assume an invalid input choice
	int userChoice = -1;
				
	// For checking return codes from functions
	int returnCode = 0;
	
	// get user choose and run simulation
	do {		
      //*************************************************************
		// Initialize factory for this test run
		shared_factory->next_in = 0;
		shared_factory->next_out = 0;
		shared_factory->widget_count = 0;
		
      //*************************************************************
		// Get the test user wants to run

        pthread_mutex_init(&count_lock, NULL);
        sem_init(&empty_slots, 0, 5);
        sem_init(&full_slots, 0, 0);

		printMenu();
		std::cin >> userChoice;
		
		switch(userChoice) {
			
         //*************************************************************
			// Case 1: Multiple producer and consumer threads
			//			Launch create threads and launch simulation
			case 1: {
				
				// Create producers
				for (int producerID = 0; producerID < MAX_PRODUCERS; producerID++) {
					pthread_create(&producer_tid[producerID], NULL, producer, &producerID);
				}
				
				// Create consumers
				for (int consumerID = 0; consumerID < MAX_CONSUMERS; consumerID++) {
					pthread_create(&consumer_tid[consumerID], NULL, consumer, &consumerID);
				}
				
				// parent sleep for the simulation after spawning threads
				printf("Parent sleeping for %d seconds to let simulation run \n", SIM_LENGTH);
				sleep(SIM_LENGTH);
				
				// Terminate producer threads
				for (int i = 0; i < MAX_PRODUCERS; i++) {
					returnCode = pthread_cancel(producer_tid[i]);
				}
				
				// Terminate consumer threads
				for (int i = 0; i < MAX_CONSUMERS; i++) {
					returnCode = pthread_cancel(consumer_tid[i]);
				}
				
            	printf(VT_RESET);
            	printf("Parent killed all children!\n");
			
				break;
						
			} // End case 1
			
			// Case 9:  // End testing
			case 9:
				moreTests = false;
				break;
			
			// Default case:  - Bad user input
			default:
				std::cout << "Please select a choice from the menu!\n";
				
		} // End test selection switch statement
		
	} while (moreTests); // end do-while
	
	// Reset terminal colors before exiting
	printf(VT_RESET);
	
	return 0;
}

//*************************************************************
//*************************************************************
// Producer Function
//*************************************************************
//*************************************************************
void* producer(void* arg)
{
	// Get ID of this Producer Thread
	int threadID = *(int *)arg;
	
	//Loop until death
	while(true) { 
			// busy-wait for open slot if buffer is full
			// while (shared_factory->widget_count == BUFFER_SIZE);
			
			// sleep to simulate time to make widget
			sleep(rand() % SLEEP_RANGE);

            sem_wait(&empty_slots);

			//place widget in buffer
			int inputSlot = shared_factory->next_in;  // save current value for later display
			shared_factory->buffer[shared_factory->next_in] = rand();

            pthread_mutex_lock(&count_lock);
			// increment widget count and next_in slot
			shared_factory->widget_count += 1;
			shared_factory->next_in = (shared_factory->next_in + 1) % BUFFER_SIZE;
            pthread_mutex_unlock(&count_lock);
			// Flip output color to RED if counters are trashed
			if ((shared_factory->widget_count < 0) || (shared_factory->widget_count > BUFFER_SIZE))
				printf(VT_RED);
			else 
				printf(VT_GREEN);
			
			// Record event to terminal	
			printf("Producer %d produced in slot %d! %d widgets now available!\n",
					threadID, inputSlot, shared_factory->widget_count);

            sem_post(&full_slots);
	} // end while forever
   pthread_exit(0);
}

//*************************************************************
//*************************************************************
// Consumer Function
//*************************************************************
//*************************************************************
void* consumer(void* arg) {
	
	// Get ID of this Consumer Thread
	int threadID = *(int *)arg;
	
	//Loop until death
	while(true)  { 
			// busy-wait for open slot if buffer is full
			// while (shared_factory->widget_count == 0);
			
			// sleep to simulate time to make widget
			sleep(rand() % SLEEP_RANGE);

            sem_wait(&full_slots);

			int outputSlot = shared_factory->next_out; // save current value for later display
			int eat_widget = shared_factory->buffer[shared_factory->next_out];

            pthread_mutex_lock(&count_lock);
			// decrent widget count and increment next_out slot
			shared_factory->widget_count -= 1;
			shared_factory->next_out = (shared_factory->next_out + 1) % BUFFER_SIZE;
        pthread_mutex_unlock(&count_lock);
			// Flip output color to RED if counters are trashed
			if ((shared_factory->widget_count < 0) || (shared_factory->widget_count > BUFFER_SIZE))
				printf(VT_RED);
			else 
				printf(VT_CYAN);

			// Record event to terminal	
			printf("* Consumer %d consumed in slot %d! %d  widgets left!\n",
					threadID, outputSlot, shared_factory->widget_count);
        sem_post(&empty_slots);
	}  // end while forever
   pthread_exit(0);
}

//*************************************************************
//*************************************************************
// Displays user input menu
//*************************************************************
//*************************************************************
void printMenu() {	
	printf(VT_BROWN);
	std::cout << "\nPlease enter which test trial you wish to run: \n";
	std::cout << "1) Spawn 5 Producer and 5 Consumer Threads\n";
	std::cout << "9) End Tests\n";
	std::cout << "\nYour selection please: ";
}

// Terminal Color Codes for Reference
/*
 \033[22;30m - black
 \033[22;31m - red
 \033[22;32m - green
 \033[22;33m - brown
 \033[22;34m - blue
 \033[22;35m - magenta
 \033[22;36m - cyan
 \033[22;37m - gray
 \033[01;30m - dark gray
 \033[01;31m - light red
 \033[01;32m - light green
 \033[01;33m - yellow
 \033[01;34m - light blue
 \033[01;35m - light magenta
 \033[01;36m - light cyan
 \033[01;37m - white
 
 */

