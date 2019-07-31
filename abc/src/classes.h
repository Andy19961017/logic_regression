#include <bitset>
#include <assert.h>
#include <algorithm>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <string>
#include <iostream>
#include <cstdlib> /* 亂數相關函數 */
#include <ctime>
#include <math.h>
#include <unordered_set>
using namespace std;

////////////////////////////////////////////////////////////////////////
///                        DECLARATIONS                              ///
////////////////////////////////////////////////////////////////////////
// #define debug

#ifdef debug
#define SENSITYVITY_TEST_MAX_ITER 10
#define SENSITYVITY_TEST_PATIENT 3
#define SENSITYVITY_TEST_SUMULATION_NUM min(2^(PI_num+3), unsigned(1)<<3)
#define CARING_PI_LIMIT 5
#else
#define SENSITYVITY_TEST_MAX_ITER 1000
#define SENSITYVITY_TEST_PATIENT 30
#define SENSITYVITY_TEST_SUMULATION_NUM min(2^(PI_num+6), unsigned(1)<<3)
#define CARING_PI_LIMIT 17
#endif

#define CH cout << "check" << endl;

#if defined(ABC_NAMESPACE)
namespace ABC_NAMESPACE
{
#elif defined(__cplusplus)
extern "C"
{
#endif

// procedures to start and stop the ABC framework
// (should be called before and after the ABC procedures are called)
void   Abc_Start();
void   Abc_Stop();

// procedures to get the ABC framework and execute commands in it
typedef struct Abc_Frame_t_ Abc_Frame_t;

Abc_Frame_t * Abc_FrameGetGlobalFrame();
int    Cmd_CommandExecute( Abc_Frame_t * pAbc, const char * sCommand );

#if defined(ABC_NAMESPACE)
}
using namespace ABC_NAMESPACE;
#elif defined(__cplusplus)
}
#endif

class PI_sensitivity { 
public:
	bool operator< (const PI_sensitivity &other) const {return sensitive_count > other.sensitive_count;} //reverse priority
    unsigned index;
    unsigned sensitive_count;
};

class PO{
public:
	bool is_sensitive(unsigned ind);
	bool is_caring_PI(unsigned ind);
	void make_sensitive(unsigned ind);
	void simulate_to_pla();
	void pla_to_aiger();
	void sort_priority_list();
	void init_caring_PI_set();
	bool in_priority_order(unsigned const & ind);
	
	PO(string n):name(n) {}
	string name;
	vector<PI_sensitivity> sensitive_list;
	vector<bool> simulated_pattern;
	unordered_set<unsigned> sensitive_set;
	unordered_set<unsigned> caring_PI_set;
};

class PI{
public:
	PI(string n):name(n) {}
	string name;
};

class simulation_handler{
public:
	// simulation_handler(char* iog, vector<PI> PIs):iogen(iog), PI_list(PIs){}
	void add_pattern(vector<bool> p) {pattern.push_back(p);}
	void simulate_by_ref();
	void add_random_pattern(unsigned const & pattern_num);
	// void simulate_by_network();
	void clear_pattern();
	void view_sensitivity_info();
	unsigned total_sensitive_PIs();


	string iogen;
	vector<vector<bool>> pattern;
	vector<string> unsat_pattern;
// private:
};

