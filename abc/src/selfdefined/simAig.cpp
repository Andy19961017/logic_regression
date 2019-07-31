#include "selfdefined.h"
#include "simMgr.h"
#include <stdio.h>
#include "misc/util/abc_global.h"
#include "base/io/ioAbc.h"
#include "base/main/main.h"
#include "misc/bbl/bblif.h"
#include <string>
#include <fstream>
/* self defined function to run simulation on AIG */
/* type sim_aig pattern_file_name to toggle this command */
ABC_NAMESPACE_IMPL_START

static Abc_Frame_t *pAbc;
/* the simulation command read by ABC framework */
int Abc_SimAig(Abc_Ntk_t * p, char* pFileName) {
	pAbc = Abc_FrameGetGlobalFrame();
	SimulationMgr Mgr;
	Mgr.bit_error_count=0;
	Mgr.initMgr(p, pFileName);
	int i = 0;
	Mgr.readPatternFile();
	while(!Mgr.patternFilereadDone()){
		i++;
		Mgr.readPattern();
		Mgr.simulateNode();
		Mgr.comparePatternwithSimulation();
	}
	std::cout << std::endl;
	std::cout << "simulate " << i*64 << " pattern" << std::endl;
	/* Abc_Ntk_t * pNtkRes; */
	/* pNtkRes = Abc_NtkToLogic( Mgr.getABCNetwork()); */
	/* Abc_FrameReplaceCurrentNetwork( pAbc, pNtkRes ); */
	/* Mgr.replaceABCNetwork(Abc_FrameReadNtk(pAbc));  */
	/* Mgr.addUnsatPattern2Network(); */
	unsigned err_count=Mgr.unSatisfiedInputPattern.size();
	std::ofstream outfile;
	outfile.open("./temp/unsat.txt");
	outfile << std::to_string(err_count) << ' ' << std::to_string(Mgr.bit_error_count) << '\n';
	for (unsigned i=0; i<err_count; ++i){
		outfile << Mgr.unSatisfiedInputPattern[i] << ' ' << Mgr.unSatisfiedOutputPattern[i] << '\n';
	}
	outfile.close();

	return 0;
}

/* the command that load unsatisfied pattern into network read by ABC framework */
int Abc_AddPatToNetwork(Abc_Ntk_t * p){
	SimulationMgr Mgr;
	if (!Mgr.isSimlated()) {
		std::cout << "Not Simulated yet" << std::endl;
		return 1;
	}
	Mgr.addUnsatPattern2Network();
	return 0;
}



/* *********************************************************** */
             /* define SimulationMgr's function */
/* *********************************************************** */
//initiailize simulation manager object
void SimulationMgr::initMgr(Abc_Ntk_t * p, char* filename){
	pNtk = p;
	nInputs = Abc_NtkPiNum(pNtk);
	nOutputs = Abc_NtkPoNum(pNtk);
	nGates = Abc_NtkNodeNum(pNtk);
	simulatePattern.resize(nInputs+nOutputs+nGates+1);
	patternFilename = filename;
}
//read pattern from pattern file
void SimulationMgr::readPattern(){
	std::vector<std::string> inputPattern;
	std::vector<std::string> outputPattern;
	std::string tempPattern;
	std::string splitPattern;
	std::bitset<64> pattern2size_t;
	/* pattern file format */
	/* inputPattern outputPattern */
	for (size_t i = 0; i < 64; i++){
		tempPattern.clear();
		splitPattern.clear();
		if(fileString.size()!=0) {
			tempPattern = fileString.back();
			fileString.pop_back();
			size_t pos = tempPattern.find(" ");
			splitPattern = tempPattern.substr(0, pos);
			inputPattern.push_back(splitPattern);
			splitPattern = tempPattern.substr(pos+1, tempPattern.size() - pos -1);
			outputPattern.push_back(splitPattern);
		}
		else break;
	}
	/* make sure that the pattern size = 64 */
	while(inputPattern.size() != 64){
		inputPattern.push_back(inputPattern.back());
		outputPattern.push_back(outputPattern.back());
	}
	/* make sure the size of the pattern is matched to the ABC current AIG */
	assert(inputPattern[0].size() == nInputs);
	assert(outputPattern[0].size() == nOutputs);
	Input.clear();
	Output.clear();
	/* convert the 64 pattern into size_t to perform parallel simulation using bitset */
	for (int i = 0; i < nInputs; i++){
		pattern2size_t.reset();
		for (size_t j = 0; j < 64; j++) {
			if(inputPattern[j][i] == '1') pattern2size_t.set(j);			
		}
		Input.push_back(pattern2size_t);
	}
	for (int i = 0; i < nOutputs; i++) {
		pattern2size_t.reset();
		for (size_t j = 0; j < 64; j++) {
			if (outputPattern[j][i] == '1') pattern2size_t.set(j);
		}
		Output.push_back(pattern2size_t);
	}
}

void SimulationMgr::simulateNode(){
	Abc_Obj_t * pNode;
	simulatePattern.clear();
	simulatePattern.resize(nInputs + nOutputs + nGates + 1);
	int i;
	std::bitset<64> const1;
	const1.set();
	simulatePattern[0] = const1;
	//ID of ABC start from 1
	Abc_NtkForEachPi( pNtk, pNode, i ){
		int Id = pNode->Id;
		simulatePattern[Id] = Input[Id-1];
	}
	Abc_NtkForEachNode( pNtk, pNode, i){
		int Id = pNode->Id;
		int fanin0ID = Abc_ObjFaninId0(pNode);
		int fanin1ID = Abc_ObjFaninId1(pNode);
		std::bitset<64> fanin0 = simulatePattern[fanin0ID];
		std::bitset<64> fanin1 = simulatePattern[fanin1ID];
		if (Abc_ObjFaninC0(pNode)) {fanin0.flip();}
		if (Abc_ObjFaninC1(pNode)) {fanin1.flip();}
		/* perform bitwise and  */
		simulatePattern[Id] = (fanin0 &= fanin1);
	}
	Abc_NtkForEachPo( pNtk, pNode, i){
		int Id = pNode->Id;
// std::cout << "\nPO Id " << Id << "\ni " << i << std::endl;
		int faninID = Abc_ObjFaninId0(pNode);
		std::bitset<64> fanin = simulatePattern[faninID];
		if (Abc_ObjFaninC0(pNode)) fanin.flip();
		simulatePattern[Id] = fanin;
	}
}

void SimulationMgr::comparePatternwithSimulation(){
	Abc_Obj_t * pNode;
	bool isDiff[64] = {false};
	// int po_count=0;
	int i=0;
	Abc_NtkForEachPo( pNtk, pNode, i){
		int Id = pNode->Id;
		std::bitset<64> comparison = (Output[i] ^= simulatePattern[Id]);
		if (comparison.none()) continue;
		else {
			for (int j = 0; j < 64; j++){
				if(comparison[j] == 1){
					isDiff[j] = true;
					bit_error_count+=1;
				}
			}
		}
	}
	std::string pattern;
	int number = 0;
	for (int i = 0; i < 64; i++){
		if (isDiff[i] == true){
			number++;
			pattern.clear();
			for (int j = 0; j < nInputs; j++){
				if (Input[j][i] == 0)
					pattern.push_back('0');
				else pattern.push_back('1');
			}
			unSatisfiedInputPattern.push_back(pattern);
			pattern.clear();
			for (int j = 0; j < nOutputs; j++){
				if (Output[j][i] == 0) pattern.push_back('0');
				else pattern.push_back('1');
			}
			unSatisfiedOutputPattern.push_back(pattern);
		}
	}

	unsigned err_count = unSatisfiedInputPattern.size();
	std::cout << '\r' << std::flush;
	std::cout << "finding " << err_count << " unsatisfied pattern" << std::flush;

}

void SimulationMgr::addUnsatPattern2Network(){	
	Abc_Obj_t * pNode;
	Abc_Obj_t * outputFanin;
	Abc_Obj_t * newFanin;
	int i;
	std::vector<Abc_Obj_t *> invertFanin;
	size_t patternNum = unSatisfiedInputPattern.size()/nInputs;
	std::cout << "add " << patternNum << " unSatisfied pattern into current network."<< std::endl;
	for (size_t j = 0; j < patternNum; j++ ){
		Vec_Ptr_t * firstFanins = Vec_PtrAlloc(nInputs);
		Vec_Ptr_t * Fanins = Vec_PtrAlloc(2);
		Abc_NtkForEachPi( pNtk, pNode, i ){
			if (unSatisfiedInputPattern[j][i] == '0') invertFanin.push_back(pNode);
			Vec_PtrPush(firstFanins, pNode);
		}
		Abc_Obj_t * newNode = Abc_NtkCreateNodeAnd(pNtk, firstFanins);
		for (size_t k = 0; k < invertFanin.size(); k++)
			Abc_NodeComplementInput(newNode, invertFanin[k]);
		Abc_NtkForEachPo( pNtk, pNode, i ){
			if (unSatisfiedOutputPattern[j][i] == '1'){
				outputFanin = Abc_ObjFanin0( pNode );
				Abc_ObjRemoveFanins( pNode );
				Vec_PtrPushTwo(Fanins, newNode, outputFanin);
				newFanin = Abc_NtkCreateNodeOr( pNtk, Fanins);
				Abc_ObjAddFanin(pNode, newFanin);
			}
		}
		Vec_PtrFree( firstFanins );
		Vec_PtrFree( Fanins );
		invertFanin.clear();
	}
}

ABC_NAMESPACE_IMPL_END
