/*

a program that imitates a clinic vaccination work flow
max 5 people in the clinic
max 2 people on the sofa
max 3 people inside the room getting vaccinated

Prob 1- race condition
Folks rushing into the clinic altogether, breaking the SOP rule.

Prob 2 - race condition
capMAX, sofaMAX, roomMAX cannot be updated correctly.

Prob 3 - synchronization
Nurse gives injections based on number of folks in the room. There is no control of which nurse is responsible for which folk, nurses might (i) simultaneously give injections to one folk; (ii) give injections to the air; (iii) some folks are not given injection at all.

*/

#include<stdio.h>
#include<unistd.h>
#include<semaphore.h>
#include<pthread.h> /* to use pthread_create() */
#define n 20 // represents the number of folks to be vaccinated

int capMAX = 0, sofaMAX = 0, roomMAX = 0, folkCount = 0, nurseCount;


void* folk(){ // go in clinic to get vaccinated
	
	int folkNum = 0;
	
	folkNum = folkCount;
	folkCount++;
	printf("\nFolk(%d) entered clinic.", folkNum);
	capMAX++;

	printf("\nFolk(%d) waiting on sofa.", folkNum);
	sofaMAX++;
	
	printf("\nFolk(%d) entered room.", folkNum);
	sofaMAX--;
	roomMAX++;
	
	printf("\nFolk(%d) says he/she is ready.", folkNum);
	sleep(0.2); // let the nurse do the job
	
	printf("\nFolk(%d) left the room and clinic.", folkNum);
	roomMAX--;
	capMAX--;
	
	if (capMAX > 5 || sofaMAX > 2 || roomMAX > 3)
		printf("\nSOP FAILED!!!");
}

void* nurse(){ // works in clinic to carry out vaccination

	int nurseNum = nurseCount++;	
	
	while (1){
		if (roomMAX > 0)
			printf("\nNurse(%d) giving injection to folk.", nurseNum);
		else
			break;
	}
	
	printf("\nNurse(%d) got off work.", nurseNum);
}

int main(){

	pthread_t folk_tid[n];
	pthread_t nurse_tid[3];
	
	int check = 0;
	
	printf("START OF PROGRAM");
	
	// creating threads
	for (int i=0; i<n; i++){ // folk threads
		// check if the thread creation is successful or not
		check = pthread_create(&folk_tid[i], NULL, &folk, NULL);
		if (check != 0)
			printf("Thread creation failed!");
	}
	
	for (int i=0; i<3; i++){ // nurse threads
		check = pthread_create(&nurse_tid[i], NULL, &nurse, NULL);
		if (check != 0)
			printf("Thread creation failed!");
	}
		
	// waiting for threads to terminate
	for (int i=0; i<n; i++) // waits for folk threads
		pthread_join(folk_tid[i], NULL);
		
	for (int i=0; i<3; i++) // waits for nurse threads
		pthread_join(nurse_tid[i], NULL);

	printf("\nEND OF PROGRAM\n");
	
	return 0;
}