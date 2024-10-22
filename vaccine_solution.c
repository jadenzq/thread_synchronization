/*

a program that imitates a clinic vaccination work flow
max 5 people in the room
max 2 people on the sofa
max 3 people on chair getting vaccinated

*/

#include<stdio.h>
#include<unistd.h>
#include<semaphore.h>
#include<pthread.h> /* to use pthread_create() */
#define QMAXSIZE 3
#define n 10 // accounts for the number of folks


sem_t capacity, sofa, room; // to avoid folks racing into the clinic at once
sem_t ready, done[n], folkWaiting; // to establish connection between nurse and folk
pthread_mutex_t capMAXLock, sofaMAXLock, roomMAXLock, folkNumQLock, nurseCountLock; // to enforce mutal exclusion

// to ensure SOP is maintained
int capMAX = 0, sofaMAX = 0, roomMAX = 0;
// to keep track of folks
int folkCount = 0, nurseCount = 0;
// queue where folk informs nurse about their number
int folkNumQ[QMAXSIZE] = {0}, front = 0, rear = -1, qSize = 0;


void enQueue(int value) {
	
	if (qSize == QMAXSIZE)
		printf("\n\nERROR: folkNumQ is Full!!");
	else {
		rear = (rear + 1) % QMAXSIZE ;
		folkNumQ[rear] = value;
		qSize++;
	}
}

int deQueue() {
	
	int temp = 0; // copy the dequeued element
	
	if (qSize == 0){
		printf("\n\nERROR: folkNumQ is Empty!!");
		return -1;
	}
	else {
		temp = folkNumQ[front];
		front = (front + 1) % QMAXSIZE;
		qSize--;
		return temp;
	}
}


void* folk(){ // go in clinic to get vaccinated

	// each folk is given a number based on the order they enter the clinic
	int folkNum = 0;
	
	if (capMAX > 5 || sofaMAX > 2 || roomMAX > 3)
		printf("\nSOP FAILED!!!");
	
	sem_wait(&capacity); // queue outside if >=5 in the clinic
	pthread_mutex_lock(&capMAXLock);
	capMAX++;
	folkNum = folkCount++; // folk gets a number upon entering the clinic
	printf("\nFolk(%d) entered clinic.", folkNum);
	pthread_mutex_unlock(&capMAXLock);
	
	sem_wait(&sofa); // wait on sofa before their turn
	pthread_mutex_lock(&sofaMAXLock);
	sofaMAX++;
	pthread_mutex_unlock(&sofaMAXLock);
	printf("\nFolk(%d) waiting on sofa.", folkNum);
	
	sem_wait(&room); // wait for empty room to get vaccinated
	pthread_mutex_lock(&roomMAXLock);
	roomMAX++;
	pthread_mutex_unlock(&roomMAXLock);
	printf("\nFolk(%d) entered room.", folkNum);
	
	pthread_mutex_lock(&sofaMAXLock);
	sofaMAX--;
	pthread_mutex_unlock(&sofaMAXLock);
	sem_post(&sofa); // enter room, return sofa
	
	// give the number to one of the nurse
	pthread_mutex_lock(&folkNumQLock);
	enQueue(folkNum);
	printf("\nFolk(%d) says he/she is ready.", folkNum);
	sem_post(&ready); // instruct the nurse that he/she is ready
	sleep(0.2);
	pthread_mutex_unlock(&folkNumQLock);
	
	
	sem_wait(&done[folkNum]); // remain in the room until the process is done
	
	printf("\nFolk(%d) left the room and clinic.", folkNum);
	pthread_mutex_lock(&roomMAXLock);
	roomMAX--;
	pthread_mutex_unlock(&roomMAXLock);
	sem_post(&room); // finish vaccination, leave room
	
	pthread_mutex_lock(&capMAXLock);
	capMAX--;
	pthread_mutex_unlock(&capMAXLock);
	sem_post(&capacity); // leave clinic
}

void* nurse(){ // works in clinic to carry out vaccination

	int noMoreFolk = 0;
	int folkNum = 0;
	int nurseNum = 0;
	
	pthread_mutex_lock(&nurseCountLock);
	nurseNum = nurseCount++;
	pthread_mutex_unlock(&nurseCountLock);
	

	while(1){
		
		// sem_trywait() return -1 if no more folks waiting
		noMoreFolk = sem_trywait(&folkWaiting);  
		if (noMoreFolk == -1){ // if all folks are done vaccinated, nurse stop working
			break;
		}
		sem_wait(&ready); // wait for folk to enter the room and be ready
		
		// take the number of folk to be vaccinated
		pthread_mutex_lock(&folkNumQLock);
		folkNum = deQueue();
		pthread_mutex_unlock(&folkNumQLock);
		
		printf("\nNurse(%d) giving injection to folk(%d).", nurseNum, folkNum);
		sem_post(&done[folkNum]); // inform the folk upon vaccination completed
	}
	
	printf("\nNurse(%d) got off work.", nurseNum);
}

int main(){

	sem_init(&capacity, 0, 5);
	sem_init(&sofa, 0, 2);
	sem_init(&room, 0, 3);
	sem_init(&ready, 0, 0);
	sem_init(&folkWaiting, 0, n);
	for (int i=0; i<n; i++){
		sem_init(&done[i], 0, 0);
	}
	
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
	
	sem_destroy(&capacity);
	sem_destroy(&sofa);
	sem_destroy(&room);
	sem_destroy(&ready);
	sem_destroy(&folkWaiting);
	for (int i=0; i<n; i++){
		sem_destroy(&done[i]);
	}
	
	printf("\nEND OF PROGRAM\n");

	return 0;
}