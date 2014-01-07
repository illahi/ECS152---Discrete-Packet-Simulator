/*--------------------------------------*/
/*    Illahi Khan & William Houston	    */
/*        995984451  & 995312942	    */
/*       ECS 152a Project Phase 1 	    */
/*--------------------------------------*/

#include <stdio.h>
#include <list>
#include <queue>
#include <stdlib.h>
#include <math.h>
#include <ctime>
using namespace std;

struct event {
	int dest; // destination host
	int sorc; // source host
	int time; // time of arrival or departure
	int type; // 0 for arrival, 1 for departure
};

struct packet {
	double serviceTime; // packet processing time
	double size; // size of packet
};

struct host {
	queue<packet> QQ; // host queue
	dropped; // dropped packet counter
	length; // queue length
	tLength; // sum of queue length over time
};

void diagnostics(void);
void insertGEL(event);
double nedt(double);	

list<event> GEL;
queue<packet> QQ;

int main()
{
	/* Randomization */
	srand48(time(NULL));

	/* User Input */
	double time		= 0;		// current time
	double mu		= 1;		// mu
	double lambda		= 0;		// lambda
	int nHosts		= 0;		// number of hosts
	double busyTime		= 0;		// total time server is busy
	int MAXBUFFER		= 10000000	// maximum queue size
	int 

	/* Initialize */
	int length = 0; /* number of packets in the queue */
	int time = 0; /* time */
	int Queue[MAXBUFFER]; /* Queue */

	/* Work */
	for (i = 0; i < 100000; i++) {
		GET first event from GEL;
		if (event is arrival) {
			process-arrival-event(eventx);
		}
		else {
			process-service-completion(eventx);
		}
	}

	/* Statistics */
	output-statistics;
	
	return 0;
}

double negative-exponentially-distributed-time(double rate)
{
	double u;
	u = drand48();
	return ((-1/rate)*log(1-u));
}


