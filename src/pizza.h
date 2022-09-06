#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>


#define Torderlow 1
#define Torderhigh 5
#define Norderlow 1
#define Norderhigh 5
#define Tpaymentlow 1
#define Tpaymenthigh 2
#define Cpizza 10
#define Pfail 0.05
#define Tprep 1
#define Tbake 10
#define Tpack 2
#define Tdellow 5
#define Tdelhigh 15

int seed;
int Npack = 1;//για τον υπάλληλο που κάνει το πακετάρισμα
int Ntel = 3;
int Ncook = 2;
int Noven = 10;
int Ndeliverer = 7;
int totalIncome=0;//συνολικά έσοδα
int failed=0;//αριθμός αποτυχημένων παραγγελιών
int succeeded=0;//αριθμός επιτυχημένων παραγγελιών

//Μέγιστοι και συνολικοί χρόνοι που ζητούνται:
int totalWaitTime=0;
int totalDeliveryTime=0;
int totalColdTime=0;
int maxWaitTime=-1;
int maxDeliveryTime=-1;
int maxColdTime=-1;

//mutexes:
pthread_mutex_t lockTel;
pthread_mutex_t lockCook;
pthread_mutex_t lockOven;
pthread_mutex_t lockPack;
pthread_mutex_t lockDel;
pthread_mutex_t lockStatistics;//για τα στατιστικά
pthread_mutex_t lockCheckPayment;//για τον έλεγχο της πληρωμής (αποτυχία ή επιτυχία παραγγελίας) και την προσθήκη income 
pthread_mutex_t lockOutput;//για την εκτύπωση

//conditions:
pthread_cond_t condDel;
pthread_cond_t condTel;
pthread_cond_t condCook;
pthread_cond_t condOven;
pthread_cond_t condPack;
