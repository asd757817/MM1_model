#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>

typedef struct {
	struct node *next;
	double it;
	double st;
	double start_wt;
	double end_wt;
	double start_syst;
	double end_syst;
} node;


typedef struct {
	node *head; 
} queue;

queue *q_new();
bool q_insert(queue *q, double it, double st);
double rand_exp(double lambda);
void calculate(queue *q);
void extract_data(queue *q);
void estimate_pn(queue *q);
double lambda,mu;
int samples;
double dt = 0;

int main(){
	srand((unsigned)time(NULL));
	lambda = 1;
	mu = 2;
	samples = 1000000;
	queue *q= q_new();

	for(int i=0;i<samples;i++){
		q_insert(q, rand_exp(lambda), rand_exp(mu));
	}
	calculate(q);
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
	//FILE *fp = fopen("test.txt", "w");
	double t = 0;
	double wt = 0; // waiting time
	double system_time = 0; // service time
	double in_queue=0; // number of customers in queue;
	double total_system_time=0; // total time in system
	double total_wt = 0;
	node *tmp = q->head;
	while(tmp){
		if(t + tmp->it >= dt){
			tmp->start_wt = t + tmp->it;
			tmp->end_wt = t + tmp->it;
			wt = 0;
		}
		else{
			tmp->start_wt = t + tmp->it;
			tmp->end_wt = dt;
			wt = dt - (t + tmp->it);
		}
		total_wt += tmp->end_wt - tmp->start_wt;
		system_time = wt + tmp->st;
		total_system_time += wt + tmp->st;
		t += tmp->it;
		dt = t + system_time;
		tmp->start_syst = t;
		tmp->end_syst = dt;
		fprintf(fp, "%f %f %f %f \n", tmp->it, tmp->st, wt, tmp->start_syst, tmp->end_syst);
		tmp = tmp->next;
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
	double b[10000];
	int n_in_system = 0;
	double t_start = 0;
	double t_end = 0;

	for(int i=0;i<50;i++){
		b[i]=0;
	}

	while(now_tmp){
		if(t_start <= now_tmp->start_syst){
			n_in_system += 1;
			t_start = now_tmp->start_syst;
		}
		node *next_tmp = now_tmp->next;
		while(next_tmp && (next_tmp->start_syst < now_tmp->end_syst)){
			if(t_start <= next_tmp->start_syst){
				t_end = next_tmp->start_syst;
				b[n_in_system] += t_end - t_start;
				n_in_system += 1;
				t_start = next_tmp->start_syst;
			}
			next_tmp = next_tmp->next;
		}
		t_end = now_tmp->end_syst;
		b[n_in_system] += t_end - t_start;
		t_start = now_tmp->end_syst;
		n_in_system -= 1;
		now_tmp = now_tmp->next;
	}

	FILE *fp = fopen("pn.txt", "w");
	double p0 = 1 - (lambda / mu);
	for(int x=1;x<=100;x++){
		double pn = p0;
		for(int i=1;i<=x;i++){
			pn *= lambda / mu;
		}
		printf("P%d -> Simulation: %f Expect: %f\n", x, b[x]/dt, pn);
		fprintf(fp, "%d %f %f \n", x, -log(b[x]/dt) , -log(pn));
	}
}
