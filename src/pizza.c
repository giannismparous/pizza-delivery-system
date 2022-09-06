#include "pizza.h"

void *order(void *x){

	int id = *(int *)x;//το id της τρέχουσας παραγγελίας/thread
	int localSeed=seed+id;//το seed που χρησιμοποιεί κάθε thread
	struct timespec start,start2,finish;//για τους χρόνους
	int pizzas;//οι πίτσες που θα λάβουν τιμή στη συνέχεια
	int delivery_customer_minutes;//χρόνος για να πάει ο ντελιβεράς στον πελάτη και επίσης χρόνος για να επιστρέψει απ'τον πελάτη
	int rc;
	
	//Τοπικοί απαιτούμενοι χρόνοι:
	int waitTime;
	int deliveryTime;
	int coldTime;
	
	if (id!=1)sleep(Torderlow+(rand_r(&localSeed))%(Torderhigh-Torderlow+1));//το πρώτο τηλεφώνημα συνδέεται κατευθείαν με τηλεφωνητή, τα υπόλοιπα δοκιμάζουν σε τυχαίο χρόνο στο δοσμένο διάστημα
	clock_gettime(CLOCK_REALTIME,&start);//στιγμή που ξεκινάει η κλήση
	
	//Αρχή κώδικα τηλεφωνητή:
	
	rc = pthread_mutex_lock(&lockTel);
	while (Ntel == 0)rc = pthread_cond_wait(&condTel, &lockTel);
	Ntel--;
	rc = pthread_mutex_unlock(&lockTel);
	
	clock_gettime(CLOCK_REALTIME,&finish);//στιγμή που τελειώνει η αναμονή κάθε κλήσης και ξεκινάει να μιλάει ο πελάτης με τον τηλεφωνητή
 	waitTime=finish.tv_sec-start.tv_sec;
	pizzas=Norderlow+rand_r(&localSeed)%(Norderhigh-Norderlow+1);
	sleep(Tpaymentlow+rand_r(&localSeed)%(Tpaymenthigh-Tpaymentlow+1));
	
	rc = pthread_mutex_lock(&lockTel);
	Ntel++;
	rc = pthread_cond_signal(&condTel);
	rc = pthread_mutex_unlock(&lockTel);
	
	//Τέλος κώδικα τηλεφωνητή και αρχή κώδικα για το αν πέτυχε η κλήση ή όχι και για την προσθήκη των εσόδων αν υπάρχουν:

	rc = pthread_mutex_lock(&lockCheckPayment);
	if (rand_r(&localSeed)%100<Pfail*100){
		failed++;
		rc = pthread_mutex_unlock(&lockCheckPayment);
		rc = pthread_mutex_lock(&lockOutput);
		printf("Η παραγγελία με αριθμό %d απέτυχε",id);
		rc = pthread_mutex_unlock(&lockOutput);
		pthread_exit(NULL);
	}
	succeeded++;
	totalIncome+=pizzas*Cpizza;
	rc = pthread_mutex_unlock(&lockCheckPayment);
	
	rc = pthread_mutex_lock(&lockOutput);
	printf("Η παραγγελία με αριθμό %d καταχωρήθηκε\n",id);
	rc = pthread_mutex_unlock(&lockOutput);
	
	//Τέλος κώδικα για το αν πέτυχε η κλήση ή όχι και για την προσθήκη των εσόδων αν υπάρχουν και αρχή κώδικα για τους ψήστες:
 
	rc = pthread_mutex_lock(&lockCook);
	while (Ncook == 0)rc = pthread_cond_wait(&condCook, &lockCook);
	Ncook--;
	rc = pthread_mutex_unlock(&lockCook);
	
	sleep(pizzas*Tprep);
	
	//Αρχή κώδικα για τους φούρνους:

	rc = pthread_mutex_lock(&lockOven);
	while (Noven < pizzas)rc = pthread_cond_wait(&condOven, &lockOven);
	Noven-=pizzas;
	rc = pthread_mutex_unlock(&lockOven);
	
	rc = pthread_mutex_lock(&lockCook);
	Ncook++;
	rc = pthread_cond_signal(&condCook);
	rc = pthread_mutex_unlock(&lockCook);
	
	sleep(Tbake);
	clock_gettime(CLOCK_REALTIME,&start2);//στιγμή που τέλειωνει το ψήσιμο
	
	//Τέλος κώδικα για τους ψήστες και αρχή κώδικα για το πακετάρισμα:

	rc = pthread_mutex_lock(&lockPack);
	while (Npack == 0 )rc = pthread_cond_wait(&condPack, &lockPack);
	Npack--;
	rc = pthread_mutex_unlock(&lockPack);
	
	sleep(Tpack);
	clock_gettime(CLOCK_REALTIME,&finish);//στιγμή που ολοκληρώθηκε το πακετάρισμα
	
	rc = pthread_mutex_lock(&lockOutput);
	printf("Η παραγγελία με αριθμό %d ετοιμάστηκε σε %.2f λεπτά\n", id,(double)(finish.tv_sec-start.tv_sec)/60);
	rc = pthread_mutex_unlock(&lockOutput);
	
	rc = pthread_mutex_lock(&lockOven);
	Noven+=pizzas;
	rc = pthread_cond_signal(&condOven);
	rc = pthread_mutex_unlock(&lockOven);
	
	rc = pthread_mutex_lock(&lockPack);
	Npack++;
	rc = pthread_cond_signal(&condPack);
	rc = pthread_mutex_unlock(&lockPack);
	
	//Τέλος κώδικα φουρνών και κώδικα για το πακετάρισμα και αρχή κώδικα παράδοσης:

	rc = pthread_mutex_lock(&lockDel);
	while (Ndeliverer == 0)rc = pthread_cond_wait(&condDel, &lockDel);
	Ndeliverer--;
	rc = pthread_mutex_unlock(&lockDel);
	
	delivery_customer_minutes=(Tdellow+rand_r(&localSeed)%(Tdelhigh-Tdellow+1));
	sleep(delivery_customer_minutes);//χρόνος για να πάει ο ντελιβεράς στον πελάτη
	clock_gettime(CLOCK_REALTIME,&finish);//στιγμή παράδοσης παραγγελίας στον πελάτη
	
	rc = pthread_mutex_lock(&lockOutput);
	printf("Η παραγγελία με αριθμό %d παραδόθηκε σε %.2f λεπτά\n", id,(double)(finish.tv_sec-start.tv_sec)/60);
	rc = pthread_mutex_unlock(&lockOutput);
	
	sleep(delivery_customer_minutes);//χρόνος για να γυρίσει ο ντελιβεράς
	
	rc = pthread_mutex_lock(&lockDel);
	Ndeliverer++;
	rc = pthread_cond_signal(&condDel);
	rc = pthread_mutex_unlock(&lockDel);
	
	//Τέλος κώδικα παράδοσης και αρχή κώδικα των στατιστικών:
	
	rc = pthread_mutex_lock(&lockStatistics);
	totalWaitTime+=waitTime;
 	if (waitTime>maxWaitTime)maxWaitTime=waitTime;
	deliveryTime=finish.tv_sec-start.tv_sec;
	totalDeliveryTime+=deliveryTime;
	if (deliveryTime>maxDeliveryTime)maxDeliveryTime=deliveryTime;
	coldTime=finish.tv_sec-start2.tv_sec;
	totalColdTime+=coldTime;
	if (coldTime>maxColdTime)maxColdTime=coldTime;
	rc = pthread_mutex_unlock(&lockStatistics);
	
	//Τέλος κώδικα στατιστικών
	
	pthread_exit(NULL);
}

int main(int argc, char* argv[]) {

	if (argc != 3) {
		printf("Σφάλμα: Απαιτούνται 2 παράμετροι, η πρώτη για το πλήθος πελατών, η δεύτερη για τον σπόρο.\n");
		exit(-1);
	}

	int Ncust = atoi(argv[1]);//αριθμός παραγγελιών
	seed = atoi(argv[2]);//σπόρος

	if (Ncust < 0) {
		printf("Σφάλμα: Ο αριθμός των πελατών είναι αρνητικός. Δοσμένος αριθμός: %d.\n", Ncust);
		exit(-1);
	}

	pthread_t* customers;//πίνακας με τα threads

	customers = malloc(Ncust * sizeof(pthread_t));//δέσμευση μνήμης customers
	
	if (customers == NULL) {
		printf("Δεν φτάνει η μνήμη για την δημιουργία του customers!\n");
		return -1;
	}
	
	int* ids=malloc(Ncust * sizeof(int));//δέσμευση μνήμης ids
	
	if (ids == NULL) {
		printf("Δεν φτάνει η μνήμη για την δημιουργία του ids!\n");
		return -1;
	}

	//initialization των mutexes και conditions:
	pthread_mutex_init(&lockOven,NULL);
	pthread_mutex_init(&lockPack, NULL);
	pthread_mutex_init(&lockCook,NULL);
	pthread_mutex_init(&lockTel, NULL);
	pthread_mutex_init(&lockDel, NULL);
	pthread_mutex_init(&lockStatistics, NULL);
	pthread_mutex_init(&lockCheckPayment, NULL);
	pthread_mutex_init(&lockOutput, NULL);
	pthread_cond_init(&condDel, NULL);
	pthread_cond_init(&condTel, NULL);
	pthread_cond_init(&condCook, NULL);
	pthread_cond_init(&condOven, NULL);
	pthread_cond_init(&condPack, NULL);
	
	int rc;
	
	for (int i = 0; i < Ncust; i++) {
		ids[i] = i+1;
		rc = pthread_create(&customers[i], NULL, order,&ids[i]);
	}
	
	for (int i = 0; i < Ncust; i++)pthread_join(customers[i], NULL);
	
	//καταστροφή των mutexes και conditions
	pthread_mutex_destroy(&lockOven);
	pthread_mutex_destroy(&lockCook);
	pthread_mutex_destroy(&lockTel);
	pthread_mutex_destroy(&lockDel);
	pthread_mutex_destroy(&lockPack);
	pthread_mutex_destroy(&lockStatistics);
	pthread_mutex_destroy(&lockCheckPayment);
	pthread_mutex_destroy(&lockOutput);
	pthread_cond_destroy(&condDel);
	pthread_cond_destroy(&condTel);
	pthread_cond_destroy(&condCook);
	pthread_cond_destroy(&condOven);
	pthread_cond_destroy(&condPack);
	
	free(customers);//αποδέσμευση μνήμης customers
	free(ids);//αποδέσμευση μνήμης ids
	
	printf("Τα συνολικά έσοδα από τις πωλήσεις είναι: %d, το πλήθος επιτυχημένων παραγγελιών είναι: %d και το πλήθος των αποτυχημένων παραγγελιών είναι: %d\n",totalIncome,succeeded,failed);
	printf("Ο μέσος χρόνος αναμονής των πελατών είναι: %.2f και ο μέγιστος χρόνος αναμονής των πελατών είναι: %.2f\n",(double)(totalWaitTime/Ncust)/60,(double)maxWaitTime/60);
	printf("Ο μέσος χρόνος εξυπηρέτησης των πελατών είναι: %.2f και ο μέγιστος χρόνος εξυπηρέτησης των πελατών είναι: %.2f\n",(double)(totalDeliveryTime/succeeded)/60,(double)maxDeliveryTime/60);
	printf("Ο μέσος χρόνος κρυώματος των παραγγελιών είναι: %.2f και ο μέγιστος χρόνος κρυώματος των παραγγελιών είναι: %.2f\n",(double)(totalColdTime/succeeded)/60,(double)maxColdTime/60);
	
	return 0;

}


