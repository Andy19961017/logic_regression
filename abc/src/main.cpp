#include "utils.h"

int main( int argc, char * argv[] )
{
    if (argc!=4){
        cout << "Usage ./main io_info.txt iogen circuit.v" << endl;
        return 0;
    }
    clock_t start = clock();
    Abc_Start();
    char* IO_info_FileName = argv[1];
    initialize(IO_info_FileName, argv[2]);
    cout << "Finding dependent PIs for each PO..." << endl;
    do_sensitivity_test();
    cout << "Constructing circuit..." << endl;
    simulate_to_pla();
    pla_to_aiger();
    join_aiger_and_simplify();
    cout << "Testing..." << endl;
    test_accuracy(1);
    technology_mapping();
    output_verilog(argv[3]);

    clock_t finish = clock();
    double used_time = double(finish-start)/CLOCKS_PER_SEC;
    cout << used_time << " second used in total" << endl;
    return 0;
}

