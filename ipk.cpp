
#include <iostream>
#include <signal.h>
#include <string>

#define ARG_ERR 1

/**
 * @brief   SIGINT handler.
 */
void sigint(int);

using namespace std;
int main(int argc, char *argv[])
{
    // argument processing
    if(argc != 3) { std::cerr << "Bad argument count.\n"; return ARG_ERR; }
    if( string(argv[1]) != "-i" ) { std::cerr << "Expected parameter -i.\n"; return ARG_ERR; }
    std::string ifce = string(argv[2]);

    //TODO

    std::cout << "Interface: " << ifce << "\n";
    return 0;
}




void sigint(int) { exit(0); }