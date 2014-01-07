/*-------------------------------*/
/* Illahi Khan & William Houston */
/*     995984451 & 995312942 	 */
/*       Phase 2 ECS 152a 	     */
/*-------------------------------*/

#include <iostream>
#include <list>
#include <queue>
#include <stdlib.h>
#include <math.h>
#include <ctime>
using namespace std;

struct event {
	int type; // 0 for arrival, 1 for departure, 2 for channel-sensing, 3 time-out event
	double time; // time of arrival or departure
	int sorc; // source host
	int dest; // destination host
	int subType; // 0 for genesis, 1 for data, 2 for ack or 0 for check, 1 for nocheck
	double size;
	int corrupt; // 0 for no, 1 for yes
};

struct packet {
	double serviceTime; // the time it takes to process
	double size; // size of the packet
};

struct host {
	queue<packet> QQ; // queue of the host
	int length; // size of the queue
	int tLength; // queue size over time total
	int dropped; // number of dropped packets
	int N;	// for backoff
	double backoff; // backoff counter
	double tTime; // transmission time
	double qTime; // queueing time
};

double nedt(double);
void insertGEL(event);
void insertTransit(event);
event newEvent(int, int, double);
void diagnostics(void);

list<event> GEL;
list<event> inTransit;

int nHosts 	= 0; // number of hosts (user-defined)
double lambda	= 0.1; // lambda (user-defined)
double mu	= 8.22/1000; // mu

int main()
{
	/* Randomization */
        srand48(time(NULL));

	/* Declarations */
	int nPackets	= 100000;	// number of packets
	int MAXBUFFER 	= 1000000;	// maximum queue size
	int totalBytes	= 0;		// number of bytes successfully transmitted
	int T		= 1;		// T-value
	double time	= 0;		// current time
	double oldTime	= 0;		// previous event time
	double tDelay	= 0;		// total delay for all hosts
	double DIFS 	= 0.1;		// distributed inter-frame space time
	double SIFS 	= 0.05;		// short inter-frame space time
	double delta	= 0.0001;	// delta

	/* User Input */
	cout << "Lambda? ";
	cin >> lambda; 
	cout << "Number of Hosts? ";
	cin >> nHosts;
	cout << "T value? ";
	cin >> T; 
	
	/* Initialization */
	host hosts[nHosts];
	for (int i = 0; i < nHosts; i++) {
		event event1 = newEvent(0, i, time);
		event1.subType = 0;
		insertGEL(event1);
		hosts[i].N = 0;
		hosts[i].length = 0;
		hosts[i].tLength = 0;
		hosts[i].dropped = 0;
		hosts[i].backoff = 0;
		hosts[i].tTime = 0;
		hosts[i].qTime = 0;
	}
	event xEvent = GEL.front();
	oldTime = xEvent.time;

	/* Iteration */
	for (int i = 0; i < nPackets; i++) {
		event currentEvent;
		currentEvent = GEL.front();
		time = currentEvent.time;

		/* Process queue arrival event */
		if (currentEvent.type == 0) {
			/* Process genesis arrival event */
			if (currentEvent.subType == 0) {
				/* Create new arrival event */
				event newArrival = newEvent(0, currentEvent.sorc, time);
				newArrival.subType = 0;
				insertGEL(newArrival);

				/* Create new packet */
				packet newPacket;
				newPacket.size = nedt(mu); // this is wrong and needs to be changed
				newPacket.serviceTime = nedt(mu); 
				/* Create departure event */
				if (hosts[currentEvent.sorc].length == 0) {
					hosts[currentEvent.sorc].QQ.push(newPacket);
					event newDeparture = newEvent(1, currentEvent.sorc, time);
					newDeparture.time = time + newPacket.serviceTime;
					newDeparture.dest = currentEvent.dest;
					newDeparture.subType = 0;
					insertGEL(newDeparture);
					hosts[currentEvent.sorc].length++;
				}
				else if (hosts[currentEvent.sorc].length > 0 && ((hosts[currentEvent.sorc].length) < MAXBUFFER)) {
					hosts[currentEvent.sorc].QQ.push(newPacket);
					hosts[currentEvent.sorc].tLength += hosts[currentEvent.sorc].length*(time-oldTime);
					hosts[currentEvent.sorc].length++;
				}
				else {
					hosts[currentEvent.sorc].dropped++;
					hosts[currentEvent.sorc].tLength += hosts[currentEvent.sorc].length*(time-oldTime);
				}
			}
			/* Process data packet arrival event */
			else if (currentEvent.subType == 1) {
				event checkEvent = inTransit.front();
				if (checkEvent.corrupt == 0) {
					/* Schedule ack packet departure event */
					event ackDeparture = newEvent(1, currentEvent.sorc, time);
					ackDeparture.subType = 1;
					ackDeparture.dest = currentEvent.dest;
					ackDeparture.size = currentEvent.size;
					ackDeparture.time = time + SIFS;
					insertGEL(ackDeparture);
				}
				else if (checkEvent.corrupt == 1) {
					// do nothing
				}
				else {
					cout << "ERROR 7 : DATA PACKET ARRIVAL" << endl;
				}
				hosts[currentEvent.sorc].tTime += (currentEvent.size/(11000000/8));
				inTransit.pop_front();
			}
			/* Process ack packet arrival event */
			else if (currentEvent.subType == 2) {
				if (inTransit.size() == 1) {
					/* Process */
					totalBytes += 64;
					totalBytes += currentEvent.size;
					/* Pop off this packet because we're done with it */
					hosts[currentEvent.sorc].QQ.pop();
					hosts[currentEvent.sorc].length--;
					/* Schedule departure event for next event in this host's queue */
					event newDeparture = newEvent(1, currentEvent.sorc, time);
					newDeparture.subType = 0;
					packet newPacket2 = hosts[currentEvent.sorc].QQ.front();
					newDeparture.time = time + newPacket2.serviceTime;
					newDeparture.dest = currentEvent.dest;
					insertGEL(newDeparture);
					hosts[currentEvent.sorc].N = 1;
				}
				else if (inTransit.size() > 1) {
					// do nothing
				}
				else {
					cout << "ERROR 10" << endl;
				}
				hosts[currentEvent.sorc].tTime += (64/(11000000/8));
				inTransit.pop_front();
			}
			else {
				cout << "ERROR 1 : ARRIVAL" << endl;
			}
		}

		/* Process queue departure event */
		else if (currentEvent.type == 1) {
			packet newPacket = hosts[currentEvent.sorc].QQ.front(); // added
			hosts[currentEvent.sorc].qTime += newPacket.serviceTime; // added
			hosts[currentEvent.sorc].tLength += hosts[currentEvent.sorc].length*(time-oldTime);
			/* Data packet departure */
			if (currentEvent.subType == 0) {
				if (hosts[currentEvent.sorc].length > 0) {
					/* Channel is free */
					if (inTransit.size() == 0) { 
						/* Create arrival event for data packet to the new host */
						event newArrival = newEvent(0, currentEvent.dest, time);
						newArrival.dest = currentEvent.sorc;
						newArrival.time = time + (newPacket.size / (11000000 / 8)) + DIFS;
						newArrival.size = newPacket.size;
						newArrival.subType = 1;
						insertGEL(newArrival);
						/* Check for corruption */
						if (inTransit.size() > 0) {
							insertTransit(newArrival);
							list<event>::iterator it;
							for (it = inTransit.begin(); it != inTransit.end(); it++) {
								it->corrupt = 1;
							}
						}
						else if (inTransit.size() == 0) {
							newArrival.corrupt = 0;
							insertTransit(newArrival);
						}
						else {
							cout << "ERROR 6: DATA CORRUPTION CHECK" << endl;
						}
								
						/* Create time-out event */
						event newTimeout = newEvent(3, currentEvent.sorc, time);
						newTimeout.dest = currentEvent.dest;
						newTimeout.time = time + SIFS + (newPacket.size / (11000000 / 8)) + (64 / (11000000 / 8)) + delta;
						insertGEL(newTimeout);
					}
					/* Channel is not free */
					else if (inTransit.size() > 0) {
						// choose random back-off value
						double temp = (rand() % 2) * T; 
						int temp1 = temp;
						if ((temp - temp1) >= 0.5) {
							hosts[currentEvent.sorc].backoff = ceil(temp);
						}
						else {
							hosts[currentEvent.sorc].backoff = floor(temp);
						}
						event channelEvent = newEvent(2, currentEvent.sorc, time);
						channelEvent.dest = currentEvent.dest;
						insertGEL(channelEvent);
					}
					else {
						cout << "ERROR 2 : DATA PACKET DEPARTURE" << endl;
					}
				}
			}
			/* Ack packet departure */
			else if (currentEvent.subType == 1) {
				/* Schedule ACK packet arrival event */
				if(inTransit.size() == 0) {
					event newArrival = newEvent(0, currentEvent.dest, time);
					newArrival.subType = 2;
					newArrival.size = currentEvent.size;
					newArrival.sorc = currentEvent.dest;
					newArrival.dest = currentEvent.sorc;
					newArrival.time = time + (64 / (11000000 / 8));
					insertGEL(newArrival);
					/* Check for corruption */
					newArrival.corrupt = 0;
					insertTransit(newArrival);
				}
				else {
					if (inTransit.size() > 0) {
						list<event>::iterator it;
						for (it = inTransit.begin(); it != inTransit.end(); it++) {
							it->corrupt = 1;
						}
					}
					else if (inTransit.size() == 0) {
						cout << "ERROR 9 : ACK PACKET DEPARTURE CHECK2" << endl;
					}
					else {
						cout << "ERROR 8 : ACK PACKET DEPARTURE CHECK" << endl;
					}
					// transmission flag is still unsuccessful
					// dont create ack arrival 
					// TO will occur and resend data packet
					// since corrupt lose this packet	
				}
			}
			else {
				cout << "ERROR 3 : DEPARTURE" << endl;
			}
					
		}

		/* Process channel-sensing event */
		else if (currentEvent.type == 2) {
			/* Channel is free */
			if (inTransit.size() == 0) {
				hosts[currentEvent.sorc].backoff -= 1;
				/* Transmit packet */
				if (hosts[currentEvent.sorc].backoff == 0) {
	        	                packet newPacket = hosts[currentEvent.sorc].QQ.front(); // added
	                	        time = currentEvent.time;
        	                	hosts[currentEvent.sorc].tLength += hosts[currentEvent.sorc].length*(time-oldTime);
					/* Create arrival event for data packet to the new host */
                                        event newArrival = newEvent(0, currentEvent.dest, time);
                                        newArrival.dest = currentEvent.sorc;
                                        newArrival.time = time + (newPacket.size / (11000000 / 8)) + DIFS;
                                        newArrival.size = newPacket.size;
                                        newArrival.subType = 1;
                                        insertGEL(newArrival);
                                        /* Check for corruption */
                                        if (inTransit.size() > 0) {
                                                insertTransit(newArrival);
                                                list<event>::iterator it;
                                                for (it = inTransit.begin(); it != inTransit.end(); it++) {
                                                        it->corrupt = 1;
                                                }
                                        }
                                        else if (inTransit.size() == 0) {
                                                newArrival.corrupt = 0;
                                                insertTransit(newArrival);
                                        }
                                        else {
                                                cout << "ERROR 6: DATA CORRUPTION CHECK" << endl;
                                        }

                                        /* Create time-out event */
                                        event newTimeout = newEvent(3, currentEvent.sorc, time);
                                        newTimeout.dest = currentEvent.dest;
                                        newTimeout.time = time + SIFS + (newPacket.size / (11000000 / 8)) + (64 / (11000000 / 8)) + delta;
                                        insertGEL(newTimeout);
				}
			}
			/* Channel is not free */
			else if (inTransit.size() > 0) {
				/* Create new channel-sensing event */
				event channelEvent = newEvent(2, currentEvent.sorc, time);
				channelEvent.dest = currentEvent.dest;
				insertGEL(channelEvent);
			}
			else {
				cout << "ERROR 9" << endl;
			}

		}

		/* Process time-out event */
		else if (currentEvent.type == 3) {
			hosts[currentEvent.sorc].N++;
			packet newPacket = hosts[currentEvent.sorc].QQ.front();
                        event newDeparture = newEvent(1, currentEvent.sorc, time);
                        newDeparture.time = time + newPacket.serviceTime;
                        newDeparture.dest = currentEvent.dest;
                        newDeparture.subType = 0;
                        insertGEL(newDeparture);
		}
		/* Throw error otherwise */
		else {
			cout << "ERROR 4" << endl;
		}
		
		GEL.pop_front();
		oldTime = time;
	}

	/* Statistics */
//	cout << "Dropped : " << dropped << endl;
//	cout << "Server Utilization : " << (busyTime / time) * 100 << "%" << endl;
//	cout << "Mean Queue Length : " << tLength / time << endl;
//	cout << "Total Bytes : " << totalBytes << endl;
	for (int i = 0; i < nHosts; i++) {
		tDelay += hosts[i].tTime + hosts[i].qTime;
	}
	cout << "Throughput : " << totalBytes / time << endl;
	cout << "Average Network Delay : " << tDelay / (totalBytes / time ) << endl;
}

void insertGEL(event newEvent)
{
	list<event>::iterator it;
	event temp;
	temp = GEL.back();

	if (newEvent.time < temp.time) {
		for (it = GEL.begin(); it != GEL.end(); it++) {
			if(newEvent.time < (it->time)) {
				GEL.insert(it, newEvent);
				break;
			}
		}
	}
	else {
		GEL.push_back(newEvent);
	}
}

void insertTransit(event newEvent)
{
        list<event>::iterator it;
        event temp;
        temp = inTransit.back();

        if (newEvent.time < temp.time) {
                for (it = inTransit.begin(); it != inTransit.end(); it++) {
                        if(newEvent.time < (it->time)) {
                                inTransit.insert(it, newEvent);
                                break;
                        }
                }
        }
        else {
                inTransit.push_back(newEvent);
        }
}

double nedt(double rate)
{
	double u;
	u = drand48();
	return ((-1/rate)*log(1-u));
}

event newEvent(int type, int sorc, double time)
{
	event temp;
	temp.type = type;
	temp.sorc = sorc;
	temp.dest = rand() % nHosts;
	while (temp.sorc == temp.dest) {
		temp.dest = rand() % nHosts;
	}
	if (type == 0) {
		temp.time = time + nedt(lambda);
	}
	else if (type == 1) {
		temp.time = time + nedt(mu);
	}
	else if (type == 2) {
		temp.time = time + 0.01;
	}
	else if (type == 3) {
		temp.time = time;
	}
	else {
		cout << "ERROR 5" << endl;
	}
	return temp;
}

void diagnostics()
{
	list<event>::iterator it;

	cout << "\n" << endl;
	cout << "GEL" << endl;
	for (it = GEL.begin(); it != GEL.end(); it++) {
		if (it->type == 0) {
			cout << "    Arrival   " << it->time << endl;
		}
		else {
			cout << "    Departure " << it->time << endl;
		}
	}
	cout << "\n" << endl;
}
