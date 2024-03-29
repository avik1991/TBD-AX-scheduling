//MUTAX WITH EQUAL TRANSMISSION POWER 
//we are trying to serve one station fully, i.e. unstil one of the stations will end its transmission
//MUTAXSO

//terminal-run command (last part for python part which is omitted now)
//gcc ax-pure_pf_sr.c -lm -lgsl -lgslcblas $(/usr/bin/python2.7-config --ldflags)

#include <err.h>
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

#include "rb-hun.c"
#include "nda.h"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

//Problems with my compiler
#define NMAX 100
#define SIFS 1.6e-5
#define AIFSN 2
#define sigma 9e-6

/* problems with my compiler
const unsigned NMAX = 100;
const double SIFS = 1.6e-5;
const double AIFSN = 2;    
const double sigma = 9e-6;

//CW = 1 always in function, bro
const unsigned CWMIN = 2;
const unsigned CWMAX = 64;
const unsigned M = 5; //MAX RETRY limit
*/
const int RBs[] = {1, 2, 4, 9, 18, 37};    			// RB widths
const int len = 5;
const int RBMAX = 18;								// Max number of RBs
const int RBMIN = 1;								// Min number of RBs
const int TONES_IN_WHOLE_CHANNEL = 484;
const double MAXSLOT = 0.005;						// 5 ms
//const double radius = 10; //10m; 37m, 100m
const double preamble = 5e-5;
const double header = 28;
const double MIN_RATE_IN_WHOLE_CHANNEL = 1.63e7;
const double AIFS = SIFS + AIFSN * sigma;			//2 2 2 3 7 --- dcf,vi,vo,be,bk

//MCS consts
const int NUMOFMCS = 12; 							// dont forget to add +1 because mcs starts from 1


//SIZE
//from 1 KB => 200 KB => 3 MB
const double MEAN = 14.28; 							//13.59 for 100 KB; 14.28 for 200 KB; 15.2 for 500 KB; 15.89 for 1MB; 18.20 for 10MB
const double FROM = 8 * 1000; 						//8 * 400
const double TO =   8 * 3 * 1e6; 					//8 * 1.5*1e6

//TIME ARRIVAL (TA)
const double TAFROM = 0.1;							//seconds
const double TAMEAN = 0.3;
const double TATO   = 0.6;

static unsigned ndstat[NMAX + 1];
static unsigned f_stat[NMAX + 1];
static unsigned rb_stat[NMAX + 1];

struct STA {
    int id;											//STA id
    int backoff;									//backoff-counter
    int r;											//retry-counter
    int da;											//da(deterministic access) or not

    double start;									//starting time
    double left;									//left size to transmit
    double size;									//size of data
    double delay_ra;								//time in ra
    double delay_aa;								//aa = ra + da
    double dist;									//STA distance from AP

    int tries;										//number of tries to transmit her own data

    //st = statistic
    double sttransmitted;
    double sttimera;
    double sttimeaa;
    double stlgtransmitted;
    double stlgtimeaa;
	
	int stflow;
	int transmissions_per_flow;
};

struct Experiment {
	int F;					// F-bound for RA
	int extra;				// number of bigger RBs in DA
	int cnt;				// number of smaller RBs in DA
	int ui;					// number of user info fields
	int nsta;				// Number of stations at start
	int nda;				// Number of stations in DA
	int frbmin;				//min frbmin
	int mode;				// 0 --- srtf, 1 --- our algorithm, 2 --- pf 3 4 5 6 --- onestafullyMUTAX (MUTAX-SO)
	int indcomeend;			// 1 - then we should recalculate everything, 0 - we are fine, skip recalculations.

	double delay;			// Total delay
	double delay_ra;		// Delay in random access
	double size;			// Mean stream size
	double transmitted;
	double time;			// time from the beginning
	double slot_duration;
	double radius;

	//unsigned long tsim;		// Simulation time
	double tsim;//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	unsigned long success;
	unsigned *schedule;		// Array with scheduler's decision
	unsigned scheduled_mcs;	// Scheduler's decision on MCS

	struct STA *stations;	// Array with STAs
	struct STA **da;		// Array with pointers to DA STAs

	double (* metric)(struct STA *, int, int, int, int, struct Experiment *);

	unsigned long morethanone;	//number of total attempts in which station was trying to transmot more than one slot


	unsigned long stslots;
	unsigned long steslots;
	unsigned long stneslots;

	unsigned long strarbcol; // ST RA RB COL - EM - SUC 
	unsigned long strarbsuc;
	unsigned long strarbem;
	unsigned long transmissions_per_flow;
	double **table;

} experiment;

gsl_rng *gen;

static int min(int a, int b) {
	if(a < b)
		return a;
	return b;
}

/* //usual CW function
static int CW(int r) {
	if(r == 0)
		return CWMIN;
	else if(r >= M)
		return CWMAX;
	return CWMIN * (1 << r);
}
*/

static int CW(int r) {
	return 1;
}

static int givetone(int rb) {
	switch(rb) {
		case 37: return 996;
		case 18: return 484;
		case 9: return 242;
		case 4: return 106;
		case 2: return 52;
		case 1: return 26;
		default: err(1, "mistake givetone %u\n", rb);
	}
}

static int modula(double P, int rb) {
	if (P >= -49 && rb >= 9) return  12;		//1024-QAM 5/6
	if (P >= -49 && rb <  9) return  10;		//256-QAM 5/6
	if (P >= -51 && rb >= 9) return  11; 		//1024-QAM 3/4
	if (P >= -51 && rb <  9) return  10; 		//256-QAM 5/6
	if (P >= -54) return  10; 					//256-QAM 5/6
	if (P >= -56) return  9; 					//256-QAM 3/4
	if (P >= -61) return  8;					//64-QAM 5/6
	if (P >= -62) return  7;					//64-QAM 3/4
	if (P >= -63) return  6;					//64-QAM 2/3
	if (P >= -67) return  5;					//16-QAM 3/4
	if (P >= -71) return  4;					//16-QAM 1/2
	if (P >= -74) return  3;					//QPSK 3/4
	if (P >= -76) return  2;					//QPSK 1/2
	if (P >= -79) return  1;					//BPSK 1/2
	return 0;
}

static double tone26(int modul) {//1
	switch(modul) {
		case 1: return 0.8;			//BPSK 1/2
		case 2: return 1.7;			//QPSK 1/2
		case 3: return 2.5;			//QPSK 3/4
		case 4: return 3.3;			//16-QAM 1/2
		case 5: return 5.0;			//16-QAM 3/4		
		case 6: return 6.7;			//64-QAM 2/3
		case 7: return 7.5;			//64-QAM 3/4
		case 8: return 8.3;			//64-QAM 5/6
		case 9: return 10.0;		//256-QAM 3/4
		case 10: return 11.1;		//256-QAM 5/6
		case 11: return 11.1; 		//^^
		case 12: return 11.1;		//^^	
		default: err(2, "mistake tone26\n");
	}
}
static double tone52(int modul) {//2
	switch(modul) {
		case 1: return 1.7;			//BPSK 1/2
		case 2: return 3.3;			//QPSK 1/2
		case 3: return 5.0;			//QPSK 3/4
		case 4: return 6.7;			//16-QAM 1/2
		case 5: return 10.0;		//16-QAM 3/4		
		case 6: return 13.3;		//64-QAM 2/3
		case 7: return 15.0;		//64-QAM 3/4
		case 8: return 16.7;		//64-QAM 5/6
		case 9: return 20.0;		//256-QAM 3/4
		case 10: return 22.2;		//256-QAM 5/6
		case 11: return 22.2; 		//^^
		case 12: return 22.2;		//^^		
		default: err(3, "mistake tone52\n");
	}
}
static double tone106(int modul) {//4
	switch(modul) {
		case 1: return 3.5;			//BPSK 1/2
		case 2: return 7.1;			//QPSK 1/2
		case 3: return 10.6;		//QPSK 3/4
		case 4: return 14.2;		//16-QAM 1/2
		case 5: return 21.3;		//16-QAM 3/4		
		case 6: return 28.3;		//64-QAM 2/3
		case 7: return 31.9;		//64-QAM 3/4
		case 8: return 35.4;		//64-QAM 5/6
		case 9: return 42.5;		//256-QAM 3/4
		case 10: return 47.2;		//256-QAM 5/6
		case 11: return 47.2; 		//^^
		case 12: return 47.2;		//^^	
		default: err(4, "mistake tone106, modul %d\n", modul);
	}
}
static double tone242(int modul) {//9
	switch(modul) {
		case 1: return 8.1;			//BPSK 1/2
		case 2: return 16.3;		//QPSK 1/2
		case 3: return 24.4;		//QPSK 3/4
		case 4: return 32.5;		//16-QAM 1/2
		case 5: return 48.8;		//16-QAM 3/4		
		case 6: return 65.0;		//64-QAM 2/3
		case 7: return 73.1;		//64-QAM 3/4
		case 8: return 81.3;		//64-QAM 5/6
		case 9: return 97.5;		//256-QAM 3/4
		case 10: return 108.3;		//256-QAM 5/6
		case 11: return 129.0; 		//1024-QAM 3/4
		case 12: return 135.4;		//1024-QAM 5/6	
		default: err(5, "mistake tone242n");
	}
}
static double tone484(int modul) {//18
	switch(modul) {
		case 1: return 16.3;		//BPSK 1/2
		case 2: return 32.5;		//QPSK 1/2
		case 3: return 48.8;		//QPSK 3/4
		case 4: return 65.0;		//16-QAM 1/2
		case 5: return 97.5;		//16-QAM 3/4		
		case 6: return 130.0;		//64-QAM 2/3
		case 7: return 146.3;		//64-QAM 3/4
		case 8: return 162.5;		//64-QAM 5/6
		case 9: return 195.0;		//256-QAM 3/4
		case 10: return 216.7;		//256-QAM 5/6	
		case 11: return 243.8;		//1024-QAM 3/4
		case 12: return 270.8; 		//1024-QAM 5/6
		default: err(6, "mistake tone484\n");
	}
}
static double tone996(int modul) {//37
	switch(modul) {
		case 1: return 34.0;		//BPSK 1/2
		case 2: return 68.1;		//QPSK 1/2
		case 3: return 102.1;		//QPSK 3/4
		case 4: return 136.1;		//16-QAM 1/2
		case 5: return 204.2;		//16-QAM 3/4		
		case 6: return 272.2;		//64-QAM 2/3
		case 7: return 306.3;		//64-QAM 3/4
		case 8: return 340.3;		//64-QAM 5/6
		case 9: return 408.3;		//256-QAM 3/4
		case 10: return 453.7;		//256-QAM 5/6	
		case 11: return 510.4;		//1024-QAM 3/4
		case 12: return 567.1; 		//1024-QAM 5/6
		default: err(7, "mistake tone996\n");
	}
}
static double gensize(double mean, double from, double to) {
	double tmp;
	do {
		tmp = gsl_ran_lognormal(gen, mean, 1);
	} while(tmp < from || tmp > to);
	return tmp;
}

static double genarrival(double mean, double from, double to) {
	if (mean == 0)
		return 0;

	double tmp;
	do {
		tmp = gsl_ran_exponential(gen, mean);
	} while(tmp < from || tmp > to);
	return tmp;
}

static double max_mcs(double dist, int rb) {
	if(rb) {
		double fc = 5.0;				//frequency 5 GHz
		double P0 = 15;        			//15dBm
		double PL = 40.05 + 20 * log10(fc/2.4) + 20 * log10(fmin(dist, 5)) + (dist > 5) * 35 * log10( dist / 5.0);
		int T = givetone(rb);  			//number of tones used.
		double P = P0 - 10 * log10( ((double) T )/ ((double) TONES_IN_WHOLE_CHANNEL)) - PL; //power
		return modula(P, rb);
	}
	return 0;
}

static int qm_mcssupported(struct STA *p, int rb, int mcs) {
	double maxmcs = max_mcs(p->dist, rb); //in this RB
	if (mcs <= maxmcs)
		return 1;
	else 
		return 0;
}

static double rate_rb_mcs(struct STA *p, int mcs, int rb) {
	if (qm_mcssupported(p, rb, mcs) == 0)
		return 0;

	if(rb && mcs) {
		int T = givetone(rb);  		//number of tones used.
		switch(T) {
			case 26: return 1e6 * tone26(mcs);
			case 52: return 1e6 * tone52(mcs);
			case 106: return 1e6 * tone106(mcs);
			case 242: return 1e6 * tone242(mcs);
			case 484: return 1e6 * tone484(mcs);
			case 996: return 1e6 * tone996(mcs);
			default: err(8, "mistake rate\n");
		}
	}
	return 0;
}
static double rate(struct STA *p, int rb) {
	int mcs = max_mcs(p->dist, rb);
	return rate_rb_mcs(p, mcs, rb);
}
static double get_slot(int edca, int ui, double rb, struct Experiment *ex){
	double ack = preamble + 14 / MIN_RATE_IN_WHOLE_CHANNEL;

	double trigger = preamble + (28 + 5 * ui + 3) / MIN_RATE_IN_WHOLE_CHANNEL; //BPSK 1/2 996 tones	
	if (ex->nda != 0) {
		ex->stneslots++;
		return trigger + SIFS + rb + SIFS + ack + AIFS;
	}
	else {
		ex->steslots++;
		return 	trigger + AIFS;
	}
}

/* Our Scheduler MUTAX*/
static double metric_ours(struct STA *p, int rb, int mcs, int index, int nsta, struct Experiment *ex) {
	return -(nsta - index) * rate_rb_mcs(p, mcs, rb) / rate(p, RBMAX);
}
static double metric_ours_test(struct STA *p, int rb, int mcs, int index, int nsta, struct Experiment *ex) {
	return -(nsta - index) * fmin(p->left, MAXSLOT*rate_rb_mcs(p, mcs, rb) / rate(p, RBMAX));
}
/* PF */
static double metric_pf(struct STA *p, int rb, int mcs, int index, int nsta, struct Experiment *ex) {
	if(p->sttransmitted)
		return -rate_rb_mcs(p, mcs, rb) * p->sttimeaa / p->sttransmitted;
	else
		return 0;
}
/* MR */
static double metric_mr(struct STA *p, int rb, int mcs, int index, int nsta, struct Experiment *ex) {
	return -rate_rb_mcs(p, mcs, rb);
}
//MUTAX-SO
static double metric_mutaxso(struct STA *p, int rb, int mcs, int index, int nsta, struct Experiment *ex) {
	double tauzzz = DBL_MAX;
	double taucandidate = 0;
	for(struct STA *p = ex->stations; p != ex->stations + ex->nsta; p++) {
		if (p->da) {
		taucandidate = p->left/ rate(p, RBMIN);
			if (taucandidate < tauzzz)
				tauzzz = taucandidate;
		}
	}
	return -(nsta - index) * fmin(p->left, tauzzz*rate_rb_mcs(p, mcs, rb) / rate(p, RBMAX));
}

static double truemutaxso_metric(struct STA *p, int rb, int mcs, int index, int nsta, struct Experiment *ex, double tauzzz) {
	return -(nsta - index) * fmin(p->left, tauzzz * rate_rb_mcs(p, mcs, rb))  / rate(p, RBMAX);
}

static void init(struct Experiment *ex) {
	ex->stations = calloc(ex->nsta, sizeof *ex->stations);
	ex->da = calloc(ex->nsta, sizeof *ex->da);
	ex->schedule = calloc(ex->nsta, sizeof *ex->schedule);
	ex->delay = 0;
	ex->delay_ra = 0;
	ex->nda = 0;
	ex->success = 0;
	ex->transmitted = 0;
	ex->time = 0;
	ex->morethanone = 0;
	ex->indcomeend = 0;
	ex->strarbcol = 0; // ST RA RB COL - EM - SUC 
	ex->strarbsuc = 0;
	ex->strarbem = 0;

	if(ex->stations == NULL)
		err(1, "Can't allocate memory for the STA array.\n");

	if(ex->da == NULL)
		err(1, "Can't allocate memory for the DA STA array.\n");

	if(ex->schedule == NULL)
		err(1, "Can't allocate memory for the schDA STA array.\n");

	switch(ex->mode) {
		case 1:	ex->metric = metric_ours_test; break;
		case 2:	ex->metric = metric_pf; break;
		case 3:	ex->metric = metric_mr; break;
		case 6: ex->metric = metric_mutaxso; break;
		default: ex->metric = NULL;
	}
	for(struct STA *p = ex->stations; p != ex->stations + ex->nsta; p++) {
		p->id = p - ex->stations;
		p->r = 0;
		p->backoff = gsl_rng_uniform_int(gen, CW(0));
		
		p->da = 0;

		p->tries = 0;
		p->size = gensize(MEAN, FROM, TO);
		p->start = genarrival(TAMEAN, TAFROM, TATO);
		p->left = p->size;

		//p->dist = ex->radius;
		
		while(1) {
			double x = gsl_rng_uniform(gen);
			double y = gsl_rng_uniform(gen);
			double l = x * x + y * y;
			if(l <= 1) {
				p->dist = sqrt(l) * ex->radius;
				break;
			}
		}
		
		/*	
		if (p->id < 8) {
			p->start = 0;
			p->dist = 20.0;
			printf(ANSI_COLOR_CYAN "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n"ANSI_COLOR_RESET);
			//printf("%f\n", p->left );
		}
		*/
		
		p->sttransmitted = 0;
		p->sttimeaa = 0;
		p->stlgtimeaa = 0;
		p->sttimera = 0;
		p->stflow = 0;
		p->stlgtransmitted = 0;
		p->transmissions_per_flow = 0;
	}
}
static void clear(struct Experiment *ex) {
	free(ex->stations);
	free(ex->da);
	free(ex->schedule);
}
static double **allocate_matrix(int n, int m) {
	cell **matrix = calloc(n, sizeof(*matrix));

	if(matrix == NULL)
		err(1, "Can't allocate memory for scheduling matrix");

	for(int i = 0; i < n; i++) {
		matrix[i] = calloc(m, sizeof(*matrix[i]));

		if(matrix[i] == NULL)
			err(1, "Can't allocate memory for scheduling matrix[%d]", i);
	}

	return matrix;
}
static void free_matrix(double **matrix, int n, int m) {
	for(int i = 0; i < n; i++)
		free(matrix[i]);
	free(matrix);
}
static void transmit(struct Experiment *ex) {
	double max_duration = 0;

	for(int i = 0; i < ex->nda; i++) {
		struct STA *p = ex->da[i];
		int rb = ex->schedule[i];
		if(!rb)
			continue;

		p->tries++;

		double transmitted = p->left;
		double r = rate_rb_mcs(p, ex->scheduled_mcs, rb);
		double duration = preamble + (header + p->left) / r;

		//printf("mcs=%d, id=%d, dist=%f, rb=%d, P=%f\n", ex->scheduled_mcs, p->id, p->dist, rb, P);

		if (duration > MAXSLOT) {
			duration = MAXSLOT;
			transmitted = (MAXSLOT - preamble) * r - header;
		}
		max_duration = fmax(max_duration, duration);
		p->left -= transmitted;
		p->sttransmitted += transmitted;
		ex->transmitted += transmitted;	
		p->transmissions_per_flow++;
	}

	//printf("NEXTPROCESS\n");

	ex->slot_duration = get_slot(0, ex->ui, max_duration, ex);

	for(struct STA *p = ex->stations; p - ex->stations != ex->nsta; p++) {
		if(p->da && p->left <= 0) {
			if (p->tries > 1)
				ex->morethanone++;

			p->tries = 0;
			ex->indcomeend = 1;
			p->stlgtransmitted += log(p->size);
			p->delay_aa = ex->time + ex->slot_duration - p->start ; 
			p->start = ex->time + ex->slot_duration + genarrival(TAMEAN, TAFROM, TATO);
			ex->delay += p->delay_aa;
			ex->delay_ra += p->delay_ra;
			p->sttimeaa += p->delay_aa;
			p->stlgtimeaa += log(p->delay_aa);
			p->stflow++;
			ex->success++;
			ex->nda--;
			p->da = 0;
			p->size = gensize(MEAN, FROM, TO);
			p->left = p->size;
			ex->transmissions_per_flow += p->transmissions_per_flow;
			p->transmissions_per_flow = 0;
		} 
	}
}

static int comp(const void *sta1, const void *sta2) {	// This function is used to sort STAs by their remaining time
	struct STA *p1 = *(struct STA **)sta1;
	struct STA *p2 = *(struct STA **)sta2;
	double m1 = p1->left / rate(p1, RBMAX);
	double m2 = p2->left / rate(p2, RBMAX);

	if(m1 < m2)
		return -1;
	else if(m1 > m2)
		return 1;

	return 0;
}

static void maximize_metric(struct Experiment *ex) {
	double bestsum_metric = DBL_MAX;			
	int index = min(ex->nda - 1, RBMAX - 1);
	int n = ex->nda;
	ex->table = allocate_matrix(n, n);
	for (int rb_set = 0; rb_set < nda_sizes[index]; rb_set++) {
		// Enumeration of all configurations
		int tmp = 0;
		int schedule_tmp[NMAX];
		int biggest_rb = 0;
		int smallest_rb = 0;
		int minmcs = 100;
		int maxmcs = 0;
		int RB_match[NMAX];

		for(int i = 0; i < len; i++) {
			tmp += RBs[i] * nda[index][rb_set][i];
			if(nda[index][rb_set][i])
				biggest_rb = RBs[i];
		}

		for(int i = 0; i < len; i++) {
			if(nda[index][rb_set][i]) {
				smallest_rb = RBs[i];
				break;
			}
		}

		if(!tmp || RBMAX - tmp < ex->frbmin)	// Do not consider assignments that leave too few RBs for random access
			continue;

		memset(RB_match, 0, sizeof(RB_match));	// RB match is about set of RBs like 1 1 1 2 2 2 9. we make some iterations over sets of this
		for(int i = 0, j = 0; i < len; i++)
			for(int k = 0; k < nda[index][rb_set][i]; k++) {
				RB_match[j++] = RBs[i];
			}

		for(int i = 0; i < n; i++) {	// Find the MCS supported by the slowest STA in the widest RB
			tmp = max_mcs(ex->da[i]->dist, biggest_rb);
			if(tmp < minmcs) {
				minmcs = tmp;
			}
		}

		for(int i = 0; i < n; i++) {	// Find the MCS supported by the slowest STA in the widest RB
			tmp = max_mcs(ex->da[i]->dist, smallest_rb);
			if(tmp > maxmcs) {
				maxmcs = tmp;
			}
		}

		for(int mcs = minmcs; mcs < maxmcs + 1; mcs++) {	// Iterate over all MCSs
			for(int i = 0; i < n; i++)						// Iterate over all RBs
				for(int j = 0; j < n; j++) 					// Iterate over all STAs
					ex->table[i][j] = ex->metric(ex->da[j], RB_match[i], mcs, j, n, ex);
			
			/*
			printf(ANSI_COLOR_GREEN "RB SET: ");
			for(int i = 0; i < n; i++) {
				printf( ANSI_COLOR_GREEN  "%d " ANSI_COLOR_RESET, RB_match[i]);
			}
			printf( ANSI_COLOR_GREEN "mcs=%d\n" ANSI_COLOR_RESET, mcs);

			
			double **tableforshow;
			tableforshow = malloc(n*sizeof(double*));
			

			for ( int i = 0; i < n; ++i ){
				tableforshow[i] = malloc(n*sizeof(double));
				memcpy(tableforshow[i], ex->table[i], n*sizeof(double));
			}*/
			
			ssize_t **assignment = kuhn_match(ex->table, n, n);		

			//print(tableforshow, n, n, assignment);	
			
			memset(schedule_tmp, 0, sizeof(schedule_tmp));
			for(int i = 0; i < n; i++) {
				int rb = assignment[i][0];
				int sta = assignment[i][1];
				schedule_tmp[sta] = RB_match[rb];
			}

			for(int i = 0; i < n; i++)
				free(assignment[i]);
			free(assignment);

			//FINDING BEST METRIC AMONG ALL CONFIGURATIONS 
			double sum = 0;
			if (ex->mode == 6) { //FOR MUTAX-SO
				sum = DBL_MAX;
				double probsum = 0; 
				double tauzzz = 0;

				for(int i = 0; i < ex->nda; i++) {
					probsum = ex->da[i]->left / rate(ex->da[i], schedule_tmp[i]);
					if (sum > probsum) {
						sum = probsum;
						tauzzz = probsum;
					}
				}

				sum = n * sum;
				
				for(int i = 0; i < ex->nda; i++) {
					sum += (n - i) * ex->da[i]->left / rate(ex->da[i], RBMAX);
				}

				for(int i = 0; i < ex->nda; i++) {
					sum += truemutaxso_metric(ex->da[i], schedule_tmp[i], mcs, i, n, ex, tauzzz);
				}

				if(sum < bestsum_metric) {
					bestsum_metric = sum;
					memcpy(ex->schedule, schedule_tmp, ex->nda * sizeof(*ex->schedule));
					ex->scheduled_mcs = mcs;
				}
			}
			else { //FOR OTHERS
				for(int i = 0; i < ex->nda; i++)
					sum += ex->metric(ex->da[i], schedule_tmp[i], mcs, i, n, ex);

				//printf( ANSI_COLOR_MAGENTA "sum = %.3f\n"  ANSI_COLOR_RESET, sum);
				if(sum < bestsum_metric) {
					bestsum_metric = sum;
					memcpy(ex->schedule, schedule_tmp, ex->nda * sizeof(*ex->schedule));
					ex->scheduled_mcs = mcs;
				}
			}
		}
	}
	
	/*
	if (ex->nda > 2) {
	for (int i = 0; i < ex->nda; i++)
		printf("[%d] = %d, ",i, ex->schedule[i]);
	printf("; mcs = %d\n", ex->scheduled_mcs);
	}
	*/

	free_matrix(ex->table, n, n);
}

static void schedule(struct Experiment *ex) {
	ex->F = RBMAX;
	ex->ui = ex->F;
	
	if (ex->nda == 0){
		ex->slot_duration = get_slot(0, ex->ui, 0, ex);
	}
	else {
		//printf(ANSI_COLOR_YELLOW "NEW TIME SLOT BOIZ, time = %.5f, indcomeend = %d\n" ANSI_COLOR_RESET, ex->time, ex->indcomeend);
		
		for(struct STA *p = ex->stations, **q = ex->da; p - ex->stations != ex->nsta; p++) // Populate the array with DA STAs
			if(p->da) {
				*q = p;
				q++;
			}

		if(ex->mode == 0 || ex->mode == 1 || ex->mode == 6)
			qsort(ex->da, ex->nda, sizeof(*ex->da), comp); // Sort STAs by the remaining time, so that the total transmission time is minimized

		if(ex->mode == 0) {
			memset(ex->schedule, 0, ex->nda * sizeof(*ex->schedule));
			ex->schedule[0] = RBMAX;
			ex->scheduled_mcs = max_mcs(ex->da[0]->dist, RBMAX);
		} 
		else if(ex->mode == 1 || ex->mode == 2 || ex->mode == 3)
			maximize_metric(ex);
		else if (ex->mode == 6) {
			if (ex->indcomeend == 1) {
				ex->indcomeend = 0;
				maximize_metric(ex);
			}
		}
		else if(ex->mode == 4) {
			double max_rate = 0;
			int winner = 0;
			for(int i = 0; i < ex->nda; i++) {
				double tmp = rate(ex->da[i], RBMAX);
				if(tmp > max_rate) {
					max_rate = tmp;
					winner = i;
				}
			}
			memset(ex->schedule, 0, ex->nda * sizeof(*ex->schedule));
			ex->schedule[winner] = RBMAX;
			ex->scheduled_mcs = max_mcs(ex->da[winner]->dist, RBMAX);
		} 
		else if(ex->mode == 5) {
			double max_rate = 0;
			int winner = 0;
			for(int i = 0; i < ex->nda; i++) {
				double tmp = rate(ex->da[i], RBMAX) * ex->da[i]->sttimeaa / ex->da[i]->sttransmitted;
				if(tmp > max_rate) {
					max_rate = tmp;
					winner = i;
				}
			}
			memset(ex->schedule, 0, ex->nda * sizeof(*ex->schedule));
			ex->schedule[winner] = RBMAX;
			ex->scheduled_mcs = max_mcs(ex->da[winner]->dist, RBMAX);

			//printf("%d ", ex->scheduled_mcs);
		} 
		else 
			err(1, "Unknown scheduler mode");

		transmit(ex);

		ex->ui = 0;
		for (int i = 0; i < ex->nda; i++)
			if(ex->schedule[i]) {
				rb_stat[ex->schedule[i]]++;
				ex->F -= ex->schedule[i];
				ex->ui++;
			}
		ex->ui += ex->F;
		f_stat[ex->F]++;
	}
}
static void request(struct Experiment *ex, unsigned long t) {
	int counter = 0;
	int cnt[RBMAX];
	int sucslots = 0;
	int colslots = 0;
	int emptyslots = 0;

	for(int i = 0; i < ex->F; i++)
		cnt[i] = 0;

	for(struct STA *p = ex->stations; p - ex->stations != ex->nsta; p++)
		if(!p->da && p->start <= ex->time) {
//			p->backoff -= ex->F;
//			if(p->backoff < 0) {
//				p->f = gsl_rng_uniform_int(gen, ex->F);
//				cnt[p->f]++;
//				counter++;
//			}
//		}
//	
//	for(struct STA *p = ex->stations; p - ex->stations != ex->nsta; p++)
//		if(!p->da && p->start <= ex->time && p->backoff < 0) {
//			if(cnt[p->f] == 1) {
				p->r = 0;
				p->tries = 0;
				p->da = 1;
				p->delay_ra = ex->time + ex->slot_duration - p->start;
				p->sttimera += p->delay_ra;
				ex->nda++;
				sucslots++;
				ex->indcomeend = 1;
//			} 
//			else if(cnt[p->f] > 1)
//				p->r++;

			p->backoff = gsl_rng_uniform_int(gen, CW(p->r));
//		}
//
//	for (int kounter = 0; kounter < ex->F; kounter++) {
//		if (cnt[kounter] > 1 ) {
//			colslots++;
//		}
	}

	emptyslots = ex->F - colslots - sucslots;

	//printf("%d %d %d %d\n", ex->F, emptyslots, colslots, sucslots);

	ex->strarbem += emptyslots;
	ex->strarbsuc += sucslots;
	ex->strarbcol += colslots;

	ex->time += ex->slot_duration;
}
static void run(struct Experiment *ex) {
	unsigned long t = 0;
	init(ex);
	while(ex->time < ex->tsim) {
		ndstat[ex->nda]++;
		schedule(ex);
		request(ex, t);
		t++;
	}
	ex->stslots = t;
	//printf("%ld\n", t);
//	printf("\r\n");
	//clear(ex);
}

int main(int argc, char **argv) {
	//printf(ANSI_COLOR_GREEN "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n"ANSI_COLOR_RESET);
	//specialtestmain();
	//testmain(argc, argv);
	unsigned long mean_active = 0, slots = 0, mean_rb = 0, mean_f = 0;
	double delay_sta = 0, transmitted;

	if(argc != 6)
		err(1, "usage: %s seed mode radius nsta tsim, 0 - SRTF, 1 - MUTAX, 2 - ax-PF, 3 - ax-MR, 4 - MR, 5 - PF, 6 - MUTAX-SO\n", argv[0]);

	gen = gsl_rng_alloc(gsl_rng_mt19937);
	if (gen == NULL)
		err(2, "Can't allocate memory for the random number generator.\n");

	gsl_rng_set(gen, atoi(argv[1]));
	experiment.mode = atoi(argv[2]);
	experiment.radius = atof(argv[3]);
	experiment.nsta = atoi(argv[4]);
	experiment.tsim = atof(argv[5]);// atol LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL

	experiment.frbmin = 0;
	run(&experiment);

	for(int i = 0; i <= NMAX; i++) {
		mean_active += i * ndstat[i];
		mean_f += i * f_stat[i];
		mean_rb += i * rb_stat[i];
		slots += ndstat[i];
	}
	
	for(int i = 0; i < experiment.nsta; i++) {
		delay_sta += experiment.stations[i].sttimeaa / experiment.stations[i].stflow;
		transmitted += experiment.stations[i].sttransmitted;
	}
	delay_sta /= experiment.nsta;
//	printf("seed\tN\tF\tT\tDRA\tD\tDSTA\tEmpty\tTransmitted\tTPF\n");
	
#ifdef DEBUG
		printf("hi,bro %f\n", 
			(double) experiment.morethanone/experiment.success);
#endif

	double trigger = preamble + (28 + 5 * 0 + 3) / MIN_RATE_IN_WHOLE_CHANNEL; //BPSK 1/2 996 tones	
	printf("\n%s\t%d\t%d\t%.3f\t%f\t%f\t%f\t%f\t%e\t%f\t%f\n",
					argv[1],
					experiment.nsta,
					experiment.frbmin,
					TAMEAN,
					(double) experiment.delay_ra / experiment.success,
					(double) experiment.delay / experiment.success,
					delay_sta,
					experiment.steslots * (trigger + AIFS) / experiment.time,
					transmitted,
					(double) experiment.transmissions_per_flow / experiment.success,
					(double) experiment.morethanone/experiment.success
					);

	gsl_rng_free(gen);
	return 0;
}
