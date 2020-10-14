#include "Shell.h"

/// Main method - the entry point of the program
///
/// The program is supposed to be run with only one
/// parameter, which is the name of the file system e.g. 'data.dat'.
/// If the user doesn't do so, an alert message will be thrown into
/// the terminal and the application closed.
///
/// \param argc number of arguments passed on from the terminal
/// \param argv values of all the arguments
/// \return 0 upon successful close of the program
int main(int argc, char *argv[]) {
    Shell shell(argc, argv);
    return 0;
}