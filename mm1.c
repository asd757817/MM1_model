#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>

// define customer structure
typedef struct {
	struct node *next;
	double it;
	double st;
	double start_wt;
	double end_wt;
	double start_syst;
	double end_syst;
} node;

// define queue
typedef struct {
	node *head; 
} queue;

queue *q_new(); // create a new queue
bool q_insert(queue *q, double it, double st); // insert node into queue
double rand_exp(double lambda); // generate exponential RV with lambda rate
void calculate(queue *q); // calculate the results
void estimate_pn(queue *q); // calculate Pn
double lambda,mu; 
int samples;
double dt = 0; //departure time;the same as total simulation time after runing all samples

int main(){
	srand((unsigned)time(NULL));
	// define parameters
	lambda = 1;
	mu = 2;
	samples = 10000000;
	queue *q= q_new();
	
	// generate customers
	for(int i=0;i<samples;i++){
		if(q_insert(q, rand_exp(lambda), rand_exp(mu)))
			continue;
		else
			printf("Inset node failed !\n");
	}

	calculate(q);

	// expected value 
	double rho = lambda / mu;
	printf("\n========Expect========\n");
	printf("Nq: %f\n", rho*rho / (1 - rho));
	printf("Ns: %f\n", rho/(1 - rho));
	printf("Average time in system: %f\n", 1/(mu - lambda));
	printf("Avergag wating time: %f\n", rho/(mu - lambda));
	printf("\n========Estimate Pn========\n");
	estimate_pn(q);

	return 0;
}



// create a queue
queue *q_new(){
	queue *q = malloc(sizeof(queue));
	if(q){
		q->head = NULL;
		return q;
	}
	else
		return NULL;
}

// add node to a queue
bool q_insert(queue *q, double it, double st){
	if(!q){
		return false;
	}
	else{
		node *tmp = malloc(sizeof(node));
		if(tmp){
			tmp->it = it;
			tmp->st = st;
			if(q->head)
				tmp->next = q->head;
			else
				tmp->next = NULL;
			q->head = tmp;
			return true;
		}
		else
			return false;
	}
}

double rand_exp(double lambda){
	double u;
	u = rand()/(RAND_MAX + 1.0);// generate random number between 0~1
	return -log(1-u)/lambda;
}

void calculate(queue *q){
	// uncomment if you want to save information to a file
	//FILE *fp = fopen("test.txt", "w");
	double t = 0;				// time variable
	double wt = 0; 				// waiting time
	double system_time = 0; 	// service time
	double in_queue=0; 			// number of customers in queue;
	double total_system_time=0; // total time in system
	double total_wt = 0; 		// total wating time

	// trace all nodes in the queue
	node *tmp = q->head;
	while(tmp){
		/* 
		   Don't need to wait,set waiting time=0, start_wait = end_wait
		   When customer enters the system,he starts to wait.
		   start_wait = the time customer entered the system.
		   t: the time last customer entered the system
		*/
		if(t + tmp->it >= dt){
			tmp->start_wt = t + tmp->it;
			wt = 0;
		}
		/*
			Need to wait. end_wait = the time last customer left the system
		*/
		else{
			tmp->start_wt = t + tmp->it;
			tmp->end_wt = dt;
			wt = dt - (t + tmp->it);
		}
		total_wt += tmp->end_wt - tmp->start_wt;	// total waiting time
		system_time = wt + tmp->st;					// the time customer stays in the system
		total_system_time += wt + tmp->st;			// total time customers stay in the system
		t += tmp->it;			// update time 
		dt = t + system_time;	// update departure time
		tmp->start_syst = t;	// the time this customer entered the system
		tmp->end_syst = dt;		// the time this customer left the system

		// uncomment if you want to record the data
		//fprintf(fp, "%f %f %f %f \n", tmp->it, tmp->st, wt, tmp->start_syst, tmp->end_syst);
		tmp = tmp->next;	// go to next node
	}

	printf("========Simulation========\n");
	printf("Average number in queue: %f\n", total_wt/dt);
	printf("Average number in system: %f\n", total_system_time/dt);
	printf("Average time in system: %f\n", total_system_time/samples);
	printf("Average wating time: %f\n", total_wt/samples);
	printf("Total Simulation Time: %f\n",dt);
}

void estimate_pn(queue *q){
	node *now_tmp = q->head;
	double b[10000];		// Set MAX N to 10000, record Pn (simulation results)
	int n_in_system = 0;	// Numbers of people in the system
	double t_start = 0;		// time interval start
	double t_end = 0;		// time interval end

	// Initialization
	for(int i=0;i<10000;i++){
		b[i]=0;
	}
	
	// trace all nodes
	while(now_tmp){
		/*
			Customer enters system ,numbers of people in system +1
			Set time interval start = the time customer entered system
		*/
		if(t_start <= now_tmp->start_syst){
			n_in_system += 1;
			t_start = now_tmp->start_syst;
		}
		/*
			Check if other customer enter system before this customer leave
		*/
		node *next_tmp = now_tmp->next;
		while(next_tmp && (next_tmp->start_syst < now_tmp->end_syst)){
			/*
				Someone enters system before this customer leave.
				Record the time between this customer entered system and next customer comes.
				(t_end = new customer's arrival time)
				Then n_in_system+1, set time interval start = the time last customer comes
			*/
			if(t_start <= next_tmp->start_syst){
				t_end = next_tmp->start_syst;
				if(n_in_system < 10000 )
					b[n_in_system] += t_end - t_start;
				n_in_system += 1;
				t_start = next_tmp->start_syst;
			}
			next_tmp = next_tmp->next;
		}
		t_end = now_tmp->end_syst;			// this customer's departing time
		if(n_in_system < 10000)
			b[n_in_system] += t_end - t_start;	// record time interval
		t_start = now_tmp->end_syst;		// move time interval
		n_in_system -= 1;					// customer left 
		now_tmp = now_tmp->next;		
	}
	// Save data to a file for plotting a graph
	FILE *fp = fopen("pn.txt", "w");
	double p0 = 1 - (lambda / mu);	// defind P0
	int N = 15; 					// set N,PN
	// Show P0 ~ PN
	for(int x=1;x<=N;x++){			
		double pn = p0;
		for(int i=1;i<=x;i++){
			pn *= lambda / mu;
		}
		printf("P%d -> Simulation: %f Expect: %f\n", x, b[x]/dt, pn);
		fprintf(fp, "%d %f %f \n", x ,b[x]/dt, pn); 
		//fprintf(fp, "%d %f %f \n", x, -log(b[x]/dt) , -log(pn)); // log scale
	}
}
