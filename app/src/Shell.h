#ifndef SHELL_H
#define SHELL_H

#include <iostream>
#include <vector>
#include <map>

#include "Logger.h"
#include "FileSystem.h"

/// \author A127B0362P silhavyj
///
/// Shell the user uses to interact with the file system (commands etc.).
///
/// This class represents a shell for the user to interact with the system.
/// All the functionality was implemented as mentioned in the document
/// available on courseware. The class also checks the syntax of the commands
/// the user types when interacting with the system. If a command does not match
/// any pre-defined pattern, the user will be informed via a message in the terminal.
class Shell {
private:
    /// enumeration of pre-defined commands the user can use
    enum CMD {
        CP,      ///< copying a file
        MV,      ///< moving a file
        RM,      ///< removing a file
        MKDIR,   ///< creating a new folder
        RMDIR,   ///< removing a folder
        LS,      ///< printing out the content of a folder
        CAT,     ///< printing out the content of a file
        CD,      ///< changing the current location within the file system
        PWD,     ///< printing out the current location (relative path)
        INFO,    ///< printing out information about an i-node
        INCP,    ///< importing a file from the HHD into the virtual file system
        OUTCP,   ///< exporting a file out of the virtual file system on to the HDD
        LOAD,    ///< loading a file from the HDD containing commands to perform on the file system
        FORMAT,  ///< formatting a new file system
        SLINK,   ///< creating a symbolic link
        HELP,    ///< printing out 'help' for the user
        EXIT,    ///< closes the program
        UNKNOWN, ///< the user entered an unknown command
        INVALID  ///< the user entered a known command in an invalid format
    };

    /// A structure for one command containing its enumeration,
    /// shortcut (what the user physically types down), simple description
    /// as what the user can use the command for, and a pointer to the function
    /// used for validation (if the syntax is correct, numbers are number, and so on).
    struct Commands {
        CMD cmd; ///< enumeration of the command
        bool (*validation)(const std::vector<std::string>& tokens); ///< pointer to the function used for validation of the command
        std::string shortcut; ///< shortcut of the command e.g. 'INCP'
        std::string desc;     ///< a short short description describing its functionality
    };

    /// Map of all the commands where the key is the shortcut so it could
    /// be used as a reference when the user types it into the terminal
    std::map<std::string, Commands> commands;

    /// Reference to the file system on which
    /// the commands the user types will be performed
    FileSystem *fileSystem = NULL;

public:
    /// Constructor of the class - creates an instance of it.
    ///
    /// This constructor takes two parameters passed on from the
    /// the header of the main method. As one of the requirements,
    /// the user is supposed to run the program with one parameter only,
    /// which is the name of the file system (e.g. data.dat). If the
    /// user doesn't do so, an alert message will be thrown and the
    /// program closed.
    ///
    /// \param argc number of arguments (argument count)
    /// \param argv arguments themselves (argument values)
    Shell(int argc, char *argv[]);

    /// Destructor of the class - deletes all dynamically allocated memory
    ///
    /// When calling this method, the program will deallocate all the memory
    /// that has been used and properly close the whole application.
    ~Shell();

private:
    /// Prints out 'help' for the user.
    ///
    /// When the user asks the program for help, they can simple type down
    /// keyword 'help' and all the commands will be printed out along with
    /// their short descriptions.
    void printHelp() const;

    /// Runs the shell in a loop.
    ///
    /// Upon successful entering of the initial parameter (the name of the file system),
    /// this method will be run in a loop. In the body of the method, the user is
    /// constantly encouraged to enter another command as what should be performed
    /// on the file system next. The method terminates when the user enters 'exit'.
    void run();

    /// Splits the string given as a parameter by the character given as a second parameter.
    ///
    /// This method is mainly used when parsing input the user just typed down.
    ///
    /// \param str string that is going to be split up byt the character
    /// \param separator character by which the string is going to be split up
    /// \return a vector of tokens
    std::vector<std::string> split(const std::string& str, char separator);

    /// Returns a command enumeration.
    ///
    /// This method analyzes the tokens given as a parameter.
    /// The first token (position 0) is meant to be the shortcut of the command e.g. 'cp'.
    /// the rest of the tokens is the arguments the user typed down along with this command,
    /// so as a whole, it may look like ['cp','file.txt','file2.txt']. The method will check
    /// if the whole command is syntactically correct. If it all seems to be okay, it will
    /// return the enumeration of the appropriate command. Otherwise, depending on the situation,
    /// it will return either #UNKNOWN or #INVALID.
    ///
    /// \param tokens command the user entered split up into separate tokens
    /// \return appropriate command enumeration including both #UNKNOWN and #INVALID
    CMD getCommand(std::vector<std::string> tokens);

    /// Reads the file on the HDD containing commands and executes them right away.
    ///
    /// This method automatizes the whole process of executing commands as the user
    /// can prepare a simple file on the HHD containing all the commands he wants to perform.
    /// The method will load the content of the file and run all the commands automatically.
    /// This could be also used for testing purposes.
    ///
    /// \param path to the file on the HDD containing commands supposed to be automatically executed
    void loadFileToExecute(std::string path);

    /// Executes a single command.
    ///
    /// The method will first take advantage of method #getCommand() in order to find out
    /// whether or not the command is valid, and if it is valid, then it will perform the command
    /// itself on the file system.
    ///
    /// \param input the entire raw line the user entered int terminal e.g. 'cp a.txt b.txt'
    /// \return true if the user entered 'exit', and therefore wants to close the application. False otherwise.
    bool executeCommand(std::string input);

    /// Tests if the line entered by the user is a valid command #CP.
    ///
    /// That includes testing such as number of parameters, values of the parameters, etc.
    /// It does not check if the file, for example, exists though. It will be taken
    /// care of later in a different part of the program - this is all about syntax.
    ///
    /// \param tokens command split up into individual tokens
    /// \return true, if the command if valid. False otherwise.
    friend bool validCP(const std::vector<std::string>& tokens);

    /// Tests if the line entered by the user is a valid command #MV.
    ///
    /// That includes testing such as number of parameters, values of the parameters, etc.
    /// It does not check if the file, for example, exists though. It will be taken
    /// care of later in a different part of the program - this is all about syntax.
    ///
    /// \param tokens command split up into individual tokens
    /// \return true, if the command if valid. False otherwise.
    friend bool validMV(const std::vector<std::string>& tokens);

    /// Tests if the line entered by the user is a valid command #RM.
    ///
    /// That includes testing such as number of parameters, values of the parameters, etc.
    /// It does not check if the file, for example, exists though. It will be taken
    /// care of later in a different part of the program - this is all about syntax.
    ///
    /// \param tokens command split up into individual tokens
    /// \return true, if the command if valid. False otherwise.
    friend bool validRM(const std::vector<std::string>& tokens);

    /// Tests if the line entered by the user is a valid command #MKDIR.
    ///
    /// That includes testing such as number of parameters, values of the parameters, etc.
    /// It does not check if the file, for example, exists though. It will be taken
    /// care of later in a different part of the program - this is all about syntax.
    ///
    /// \param tokens command split up into individual tokens
    /// \return true, if the command if valid. False otherwise.
    friend bool validMKDIR(const std::vector<std::string>& tokens);

    /// Tests if the line entered by the user is a valid command #RMDIR.
    ///
    /// That includes testing such as number of parameters, values of the parameters, etc.
    /// It does not check if the file, for example, exists though. It will be taken
    /// care of later in a different part of the program - this is all about syntax.
    ///
    /// \param tokens command split up into individual tokens
    /// \return true, if the command if valid. False otherwise.
    friend bool validRMDIR(const std::vector<std::string>& tokens);

    /// Tests if the line entered by the user is a valid command #LS.
    ///
    /// That includes testing such as number of parameters, values of the parameters, etc.
    /// It does not check if the file, for example, exists though. It will be taken
    /// care of later in a different part of the program - this is all about syntax.
    ///
    /// \param tokens command split up into individual tokens
    /// \return true, if the command if valid. False otherwise.
    friend bool validLS(const std::vector<std::string>& tokens);

    /// Tests if the line entered by the user is a valid command #CAT.
    ///
    /// That includes testing such as number of parameters, values of the parameters, etc.
    /// It does not check if the file, for example, exists though. It will be taken
    /// care of later in a different part of the program - this is all about syntax.
    ///
    /// \param tokens command split up into individual tokens
    /// \return true, if the command if valid. False otherwise.
    friend bool validCAT(const std::vector<std::string>& tokens);

    /// Tests if the line entered by the user is a valid command #CD.
    ///
    /// That includes testing such as number of parameters, values of the parameters, etc.
    /// It does not check if the file, for example, exists though. It will be taken
    /// care of later in a different part of the program - this is all about syntax.
    ///
    /// \param tokens command split up into individual tokens
    /// \return true, if the command if valid. False otherwise.
    friend bool validCD(const std::vector<std::string>& tokens);

    /// Tests if the line entered by the user is a valid command #PWD.
    ///
    /// That includes testing such as number of parameters, values of the parameters, etc.
    /// It does not check if the file, for example, exists though. It will be taken
    /// care of later in a different part of the program - this is all about syntax.
    ///
    /// \param tokens command split up into individual tokens
    /// \return true, if the command if valid. False otherwise.
    friend bool validPWD(const std::vector<std::string>& tokens);

    /// Tests if the line entered by the user is a valid command #INFO.
    ///
    /// That includes testing such as number of parameters, values of the parameters, etc.
    /// It does not check if the file, for example, exists though. It will be taken
    /// care of later in a different part of the program - this is all about syntax.
    ///
    /// \param tokens command split up into individual tokens
    /// \return true, if the command if valid. False otherwise.
    friend bool validINFO(const std::vector<std::string>& tokens);

    /// Tests if the line entered by the user is a valid command #INCP.
    ///
    /// That includes testing such as number of parameters, values of the parameters, etc.
    /// It does not check if the file, for example, exists though. It will be taken
    /// care of later in a different part of the program - this is all about syntax.
    ///
    /// \param tokens command split up into individual tokens
    /// \return true, if the command if valid. False otherwise.
    friend bool validINCP(const std::vector<std::string>& tokens);

    /// Tests if the line entered by the user is a valid command #OUTCP.
    ///
    /// That includes testing such as number of parameters, values of the parameters, etc.
    /// It does not check if the file, for example, exists though. It will be taken
    /// care of later in a different part of the program - this is all about syntax.
    ///
    /// \param tokens command split up into individual tokens
    /// \return true, if the command if valid. False otherwise.
    friend bool validOUTCP(const std::vector<std::string>& tokens);

    /// Tests if the line entered by the user is a valid command #LOAD.
    ///
    /// That includes testing such as number of parameters, values of the parameters, etc.
    /// It does not check if the file, for example, exists though. It will be taken
    /// care of later in a different part of the program - this is all about syntax.
    ///
    /// \param tokens command split up into individual tokens
    /// \return true, if the command if valid. False otherwise.
    friend bool validLOAD(const std::vector<std::string>& tokens);

    /// Tests if the line entered by the user is a valid command #FORMAT.
    ///
    /// That includes testing such as number of parameters, values of the parameters, etc.
    /// It does not check if the file, for example, exists though. It will be taken
    /// care of later in a different part of the program - this is all about syntax.
    ///
    /// \param tokens command split up into individual tokens
    /// \return true, if the command if valid. False otherwise.
    friend bool validFORMAT(const std::vector<std::string>& tokens);

    /// Tests if the line entered by the user is a valid command #EXIT.
    ///
    /// That includes testing such as number of parameters, values of the parameters, etc.
    /// It does not check if the file, for example, exists though. It will be taken
    /// care of later in a different part of the program - this is all about syntax.
    ///
    /// \param tokens command split up into individual tokens
    /// \return true, if the command if valid. False otherwise.
    friend bool validEXIT(const std::vector<std::string>& tokens);

    /// Tests if the line entered by the user is a valid command #HELP.
    ///
    /// That includes testing such as number of parameters, values of the parameters, etc.
    /// It does not check if the file, for example, exists though. It will be taken
    /// care of later in a different part of the program - this is all about syntax.
    ///
    /// \param tokens command split up into individual tokens
    /// \return true, if the command if valid. False otherwise.
    friend bool validHELP(const std::vector<std::string>& tokens);

    /// Tests if the line entered by the user is a valid command #SLINK.
    ///
    /// That includes testing such as number of parameters, values of the parameters, etc.
    /// It does not check if the file, for example, exists though. It will be taken
    /// care of later in a different part of the program - this is all about syntax.
    ///
    /// \param tokens command split up into individual tokens
    /// \return true, if the command if valid. False otherwise.
    friend bool validSLINK(const std::vector<std::string>& tokens);

    /// Tests if the string given as a parameter is consist of digits only.
    /// \param str string in which we want to check if there are only digits (0 - 9) in it.
    /// \return true if the string contains only digits. False otherwise.
    friend bool containsOnlyDigits(std::string str);
};

#endif