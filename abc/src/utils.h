#include "classes.h"


vector<PO> PO_list;
vector<PI> PI_list;
string PI_names="";
unsigned PI_num=0;
unsigned PO_num=0;
simulation_handler sh;
Abc_Frame_t * pAbc;


void set_alias();

// bool PO::not_in_insensitive_list(unsigned ind){
	// return !(find(insensitive_list.begin(), insensitive_list.end(), ind) != insensitive_list.end());
// }

bool PO::is_sensitive(unsigned ind){
	return sensitive_set.find (ind) != sensitive_set.end();
}

void PO::init_caring_PI_set(){
	caring_PI_set.clear();
	unsigned num=min(CARING_PI_LIMIT, int(sensitive_set.size()));
	for (unsigned i=0; i<num; ++i){
		caring_PI_set.insert(sensitive_list[i].index);
	}
}


void PO::sort_priority_list(){
	sort(sensitive_list.begin(), sensitive_list.end());
// cout << "sort_priority_list" << endl;
// for (unsigned i=0;i<PI_num;++i){
	// cout << sensitive_list[i].index << " " << sensitive_list[i].sensitive_count << endl;
// }
}

void PO::make_sensitive(unsigned ind){
	sensitive_list[ind].sensitive_count+=1;
	if (!is_sensitive(ind)){
		sensitive_set.insert(ind);
	}
}

// void PO::simulate_to_pla(){
// 	sh.clear_pattern();
// 	// unsigned const digits=min(int(sensitive_set.size()), CARING_PI_LIMIT);
// 	unsigned digits=min(int(sensitive_set.size()), CARING_PI_LIMIT);

// 	string pattern;
// 	size_t total=size_t(1)<<digits;
// 	for (size_t i=0; i<total; ++i){
// 		pattern=bitset<CARING_PI_LIMIT>(i).to_string();
// 		pattern=pattern.substr (CARING_PI_LIMIT-digits,digits); 
// 		vector<bool> simulated_pattern;
// // cout << "pattern " << pattern << " digits " << digits << endl;
// 		for (unsigned j=0; j<PI_num; ++j){
// 			if (is_caring_PI(j)){
// 				simulated_pattern.push_back(pattern.back()=='1');
// 				pattern.pop_back();
// // cout << "pattern " << pattern << endl;
// 			}
// 			else{
// // cout << "rand!" << endl;
// 				simulated_pattern.push_back(rand()%2);
// 			}
// 		}
// // cout << "simulated_pattern" << simulated_pattern[0] << simulated_pattern[1] << simulated_pattern[2] << simulated_pattern[3] << simulated_pattern[4] << simulated_pattern[5] << endl;
// 		sh.pattern.push_back(simulated_pattern);
// 	}
// 	sh.simulate_by_ref();
// 	ofstream pla;
// 	pla.open("./temp/"+name+".pla");
// 	pla << ".i " << to_string(PI_num) << '\n';
// 	pla << ".o 1" << '\n';
// 	pla << ".ilb" << PI_names << '\n';
// 	pla << ".ob " << name << '\n';
// 	pla << ".p " << to_string(total) << '\n';	
// 	for (unsigned i=0; i<total; ++i){
// 		for (unsigned j=0; j<PI_num; ++j){
// 			pla << (is_caring_PI(j) ? (sh.pattern[i][j] ? '1' : '0') : '-' );
// 			// pla << (is_sensitive(j) ? (sh.pattern[i][j] ? '1' : '0') : '-' );
// 			// pla << (is_sensitive(j) ? (sh.pattern[i][j] ? '1' : '0') : (sh.pattern[i][j] ? '1' : '0') );
// 		}
// 		pla << ' ' << (simulated_pattern[i] ? '1' : '0') << '\n';
// 	}
// 	pla << ".e\n";
// 	pla.close();
// }
bool PO::in_priority_order(unsigned const & ind){
	for (unsigned i=0; i<CARING_PI_LIMIT; ++i){
		if (sensitive_list[i].index==ind) return true;
	}
	return false;
}


void PO::simulate_to_pla(){
	sh.clear_pattern();
	// unsigned const digits=min(int(sensitive_set.size()), CARING_PI_LIMIT);
	unsigned digits=min(unsigned(CARING_PI_LIMIT), PI_num);

	string pattern;
	size_t total=size_t(1)<<digits;
	for (size_t i=0; i<total; ++i){
		pattern=bitset<CARING_PI_LIMIT>(i).to_string();
		pattern=pattern.substr (CARING_PI_LIMIT-digits,digits); 
		vector<bool> simulated_pattern;
		for (unsigned j=0; j<PI_num; ++j){
			if (in_priority_order(j)){
				simulated_pattern.push_back(pattern.back()=='1');
				pattern.pop_back();
			}
			else{
				simulated_pattern.push_back(rand()%2);
			}
		}
		sh.pattern.push_back(simulated_pattern);
	}
	sh.simulate_by_ref();
	ofstream pla;
	pla.open("./temp/"+name+".pla");
	pla << ".i " << to_string(PI_num) << '\n';
	pla << ".o 1" << '\n';
	pla << ".ilb" << PI_names << '\n';
	pla << ".ob " << name << '\n';
	pla << ".p " << to_string(total) << '\n';	
	for (unsigned i=0; i<total; ++i){
		for (unsigned j=0; j<PI_num; ++j){
			// pla << (is_caring_PI(j) ? (sh.pattern[i][j] ? '1' : '0') : '-' );
			pla << (in_priority_order(j) ? (sh.pattern[i][j] ? '1' : '0') : '-' );
			// pla << (is_sensitive(j) ? (sh.pattern[i][j] ? '1' : '0') : '-' );
			// pla << (is_sensitive(j) ? (sh.pattern[i][j] ? '1' : '0') : (sh.pattern[i][j] ? '1' : '0') );
		}
		pla << ' ' << (simulated_pattern[i] ? '1' : '0') << '\n';
	}
	pla << ".e\n";
	pla.close();
}

bool PO::is_caring_PI(unsigned ind){
	return caring_PI_set.find (ind) != caring_PI_set.end();
}


void PO::pla_to_aiger(){
	Cmd_CommandExecute(pAbc, ("read_pla ./temp/"+name+".pla").c_str());
	Cmd_CommandExecute(pAbc, "strash");
	cout << "Status for single PI circuit (" << name << "): ";
	Cmd_CommandExecute(pAbc, "print_stats");
	// Cmd_CommandExecute(pAbc, "source ./scripts/simplify_individual.txt");
	Cmd_CommandExecute(pAbc, ("write_aiger -s ./temp/"+name+".aig").c_str());
}

// unsigned PO::get_correct_pattern_num(){
// 	unsigned l=simulated_pattern.size();
// 	unsigned count=0;
// 	for (unsigned i=0; i<l; ++i){
// 		if (simulated_pattern[i]==network_simulated_pattern[i]){
// 			++count;
// 		}
// 	}
// 	return count;
// }

void simulation_handler::simulate_by_ref(){
	ofstream in_pat;
	in_pat.open("./temp/in_pat.txt");
	in_pat << to_string(PI_num) << ' ' << to_string(pattern.size()) << '\n';
	for (unsigned i=0; i<PI_num-1; ++i){
		in_pat << PI_list[i].name << ' ';
	}
	in_pat << PI_list[PI_num-1].name << '\n';
	unsigned num_of_pat=pattern.size();
	for (unsigned i=0; i<num_of_pat; ++i){
		for (unsigned j=0; j<PI_num-1; ++j){
			in_pat << (pattern[i][j] ? '1' : '0') << ' ';
		}
		in_pat << (pattern[i][PI_num-1] ? '1' : '0') << '\n';
	}
	in_pat.close();

	system((iogen+" ./temp/in_pat.txt ./temp/io_rel.txt").c_str());
	ifstream io_rel;
	string line;
	io_rel.open("./temp/io_rel.txt");
	getline(io_rel, line);
	getline(io_rel, line);
	while(!io_rel.eof()){
		getline(io_rel, line);
		if (line=="") break;
		for (unsigned i=0; i<PO_num; ++i){
			PO_list[i].simulated_pattern.push_back(line[(PI_num+i)*2]=='1');
		}
	}
	io_rel.close();
}

void simulation_handler::view_sensitivity_info(){
	cout << "\r" << flush;
	cout << "Num of dependent PIs for each PO:";
	for (unsigned i=0; i<PO_num; ++i){
		cout << " " << PO_list[i].sensitive_set.size();
	}
	cout << ", total_PIs: " << PI_num << flush;
}

unsigned simulation_handler::total_sensitive_PIs(){
	unsigned sum=0;
	for (unsigned i=0; i<PO_num; ++i){
		sum+=PO_list[i].sensitive_set.size();
	}
	return sum;
}

void simulation_handler::clear_pattern(){
	pattern.clear();
	for (unsigned i=0; i<PO_num; ++i){
		PO_list[i].simulated_pattern.clear();
	}
}

void simulation_handler::add_random_pattern(unsigned const & pattern_num){
	assert(pattern.empty());
	for (unsigned j=0; j<pattern_num; ++j){
		vector<bool> pat;
		for (unsigned k=0; k<PI_num; ++k){
			pat.push_back(bool(rand()%2));
		}
		pattern.push_back(pat);
	}
}

void read_io_info(vector<PI>& PI_list, vector<PO>& PO_list, char * const IO_info_FileName){
	ifstream IO_file(IO_info_FileName);
	string line;
	if (IO_file.is_open()){
		getline(IO_file, line);
		unsigned i=0;
		for (; line[i]!=' '; i++){
			PI_num=10*PI_num+(line[i]-'0');
		}
		i++;
		for (; i<line.length(); i++){
			PO_num=10*PO_num+(line[i]-'0');
		}
		getline(IO_file, line);
		i=0;
		unsigned l=line.length();
		unsigned PI_count=0;
		unsigned ptr=0;
		for (; i<=l; i++){
			if (line[i]==' ' or i==l){
				if (PI_count<PI_num){
					PI_list.push_back(PI(line.substr(ptr, i-ptr)));
					ptr=i+1;
					PI_count++;
				}
				else{
					PO_list.push_back(PO(line.substr(ptr, i-ptr)));
					ptr=i+1;
				}
			}
		}
	}
}

void mk_temp_dir(){
	const int dir_err = system("mkdir -p temp");
	if (-1 == dir_err){
		system("rm -r temp");
		system("mkdir -p temp");
	}
}

void initialize(char * const IO_info_FileName, char* const iogen){
	read_io_info(PI_list, PO_list, IO_info_FileName);
    vector<PI_sensitivity> temp;
    for (unsigned i=0; i<PI_num; ++i){
    	PI_sensitivity p;
    	p.index=i;
    	p.sensitive_count=0;
    	temp.push_back(p);
        PI_names+=" "+PI_list[i].name;
    }
    for (unsigned i=0; i<PO_num; ++i){
    	PO_list[i].sensitive_list=temp;
    }
    //initialize similation handler
    sh.iogen=string(iogen);
	srand(time(NULL));
	mk_temp_dir();
    pAbc = Abc_FrameGetGlobalFrame();
    set_alias();
}

bool rand_bin(){
	return rand()%2;
}

void simulate_and_test(){
	sh.clear_pattern();
	unsigned pattern_num = SENSITYVITY_TEST_SUMULATION_NUM;
	for (unsigned i=0; i<PI_num; ++i){ // the compared PI
		for (unsigned j=0; j<pattern_num; ++j){
			vector<bool> pat;
			for (unsigned k=0; k<PI_num; ++k){
				pat.push_back(bool(rand()%2));
			}
			sh.add_pattern(pat);
			pat[i]=!pat[i];
			sh.add_pattern(pat);
		}
	}
	sh.simulate_by_ref();
	for (unsigned i=0; i<PO_num; ++i){
		for (unsigned j=0; j<PI_num; ++j){
			for (unsigned k=0; k<pattern_num; ++k){
				int ind=j*pattern_num+k;
				if (PO_list[i].simulated_pattern[2*ind]!=PO_list[i].simulated_pattern[2*ind+1]){
					PO_list[i].make_sensitive(j);
				}
				break;
			}
		}
	}
}

void do_sensitivity_test(){
	unsigned patient=SENSITYVITY_TEST_PATIENT;
	unsigned p=patient;
	unsigned count=0;
	unsigned total_PIs=sh.total_sensitive_PIs();
	while(count<SENSITYVITY_TEST_MAX_ITER && p>0){
		simulate_and_test();
		unsigned new_total_PIs=sh.total_sensitive_PIs();
		if (!(new_total_PIs > total_PIs)){ // no inprovement
			p-=1;
		}
		else{
			p=patient;
			total_PIs=new_total_PIs;
			cout << " ";
			sh.view_sensitivity_info();	
		}
		count+=1;
	}
	for (unsigned i=0; i<PO_num; ++i){
		PO_list[i].sort_priority_list();
		PO_list[i].init_caring_PI_set();
	}
	cout << endl;
}

void simulate_to_pla(){
	for (unsigned i=0; i<PO_num; ++i){
		PO_list[i].simulate_to_pla();
	}	
}

void pla_to_aiger(){
	for (unsigned i=0; i<PO_num; ++i){
		PO_list[i].pla_to_aiger();
	}
}

void set_alias(){
	Cmd_CommandExecute(pAbc, "source abc.rc");
}

void join_aiger_and_simplify(){
	Cmd_CommandExecute(pAbc, ("read_aiger ./temp/"+PO_list[0].name+".aig").c_str());
	Cmd_CommandExecute(pAbc, "strash");
	for (unsigned i=1; i<PO_num; ++i){
		Cmd_CommandExecute(pAbc, ("append ./temp/"+PO_list[i].name+".aig").c_str());
	}
	cout << "Status for multi PI circuit: ";
	Cmd_CommandExecute(pAbc, "print_stats");
	// Cmd_CommandExecute(pAbc, "source ./scripts/simplify_joint.txt");
}

void output_verilog(char* const f){
	string filename(f);
	Cmd_CommandExecute(pAbc, ("write_verilog "+filename).c_str());
	cout << filename << " is saved." << endl;
}

void create_pattern_file(){
	ifstream infile;
	ofstream outfile;
	infile.open("./temp/io_rel.txt");
	outfile.open("./temp/pattern.pat");
	string line;
	getline(infile,line);
	getline(infile,line);
	while(!infile.eof()){
		string line;
		getline(infile,line);
		if (line.length()!=2*(PI_num+PO_num)-1) continue;
		for (unsigned i=0; i<PI_num; ++i){
			outfile << line[2*i];
		}
		outfile << ' ';
		for (unsigned i=0; i<PO_num; ++i){
			outfile << line[2*(PI_num+i)];
		}		
		outfile << '\n';
	}
	infile.close();
	outfile.close();
}

void test_accuracy(bool const & get_pattern){
	sh.unsat_pattern.clear();
	unsigned total=5000;
	sh.clear_pattern();
	sh.add_random_pattern(total);
	sh.simulate_by_ref();
	create_pattern_file();
	Cmd_CommandExecute(pAbc, "sim_aig ./temp/pattern.pat");
	ifstream unsat;
	unsat.open("./temp/unsat.txt");
	string line;
	getline(unsat, line);
	size_t pos = line.find(" ");
	size_t pos2 = line.find('\n');
	string bit_err_count=line.substr(pos+1,pos2);
	if (get_pattern){
		while(!unsat.eof()){
			getline(unsat, line);
			if (line=="") continue;
			sh.unsat_pattern.push_back(line);
		}
	}
	if (total/64){
		total=((total/64)+1)*64;
	}
	cout << "Accuracy: " << 1-double(stoi(bit_err_count))/(total*PO_num) << endl;
}

void technology_mapping(){
	Cmd_CommandExecute(pAbc, "read_library ../lib.sic");
	Cmd_CommandExecute(pAbc, "map");
}