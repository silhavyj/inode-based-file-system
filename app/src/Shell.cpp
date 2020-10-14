#include "Shell.h"

// function prototypes
bool validCP(const std::vector<std::string>& tokens);
bool validMV(const std::vector<std::string>& tokens);
bool validRM(const std::vector<std::string>& tokens);
bool validMKDIR(const std::vector<std::string>& tokens);
bool validRMDIR(const std::vector<std::string>& tokens);
bool validLS(const std::vector<std::string>& tokens);
bool validCAT(const std::vector<std::string>& tokens);
bool validCD(const std::vector<std::string>& tokens);
bool validPWD(const std::vector<std::string>& tokens);
bool validINCP(const std::vector<std::string>& tokens);
bool validOUTCP(const std::vector<std::string>& tokens);
bool validINFO(const std::vector<std::string>& tokens);
bool validLOAD(const std::vector<std::string>& tokens);
bool validFORMAT(const std::vector<std::string>& tokens);
bool validEXIT(const std::vector<std::string>& tokens);
bool validHELP(const std::vector<std::string>& tokens);
bool validSLINK(const std::vector<std::string>& tokens);

bool containsOnlyDigits(std::string str);

Shell::Shell(int argc, char *argv[]) {
    // fill out the table of commands so it can
    // be used for validation, printing out help, and so on
    commands["cp"]     = {CP,     &validCP,     "cp s1 s2",     "- copies file s1 into file s2"};
    commands["mv"]     = {MV,     &validMV,     "mv s1 s2",     "- moves file s1 into file s2"};
    commands["rm"]     = {RM,     &validRM,     "rm s1",        "- removes file s1"};
    commands["mkdir"]  = {MKDIR,  &validMKDIR,  "mkdir a1",     "- creates a new folder a1"};
    commands["rmdir"]  = {RMDIR,  &validRMDIR,  "rmdir a1",     "- removes folder a1"};
    commands["ls"]     = {LS,     &validLS,     "ls a1",        "- prints out the content of folder a1"};
    commands["cat"]    = {CAT,    &validCAT,    "cat s1",       "- prints out the content of file s1"};
    commands["cd"]     = {CD,     &validCD,     "cd a1",        "- changes the current path into folder a1"};
    commands["pwd"]    = {PWD,    &validPWD,    "pwd",          "- prints out the current path"};
    commands["incp"]   = {INCP,   &validINCP,   "incp s1 s2",   "- load file s1 into the file system (directory s2)"};
    commands["outcp"]  = {OUTCP,  &validOUTCP,  "outcp s1 s2",  "- exports file s1 out onto the physical disk (directory s2)"};
    commands["info"]   = {INFO,   &validINFO,   "info a1/s1",   "- prints out information about the i-node"};
    commands["load"]   = {LOAD,   &validLOAD,   "load s1",      "- loads commands stored in file s1 and executes them"};
    commands["format"] = {FORMAT, &validFORMAT, "format 600MB", "- formats the file given as a parameter"};
    commands["slink"]  = {SLINK,  &validSLINK,  "slink s1 s2",  "- creates a symbolic link s2 pointing at file s1"};
    commands["help"]   = {HELP,   &validHELP,   "help",         "- prints out help"};
    commands["exit"]   = {EXIT,   &validEXIT,   "exit",         "- closes the application"};

    // test if the user run the program with
    // one parameter which happens to be the
    // name of the file system
    if (argc != 2)
        std::cout << "You are supposed to run the program with one parameter, which is the name of the file system (e.g. data.dat).\n";
    else {
        // if everything's okay - create a file system
        // and run the loop where the user enters commands
        std::string diskFileName(argv[1]);
        fileSystem = new FileSystem(diskFileName);
        run();
    }
}

Shell::~Shell() {
    // delete the file system
    if (fileSystem != NULL)
        delete fileSystem;
}

void Shell::run() {
    std::string input;
    while (1) {
        // encourage  the user to enter a command by
        // printing out the input line (pwd>)
        std::cout << fileSystem->getCurrentPath() << "> ";
        std::getline(std::cin, input);
        if (executeCommand(input))
            return;
    }
}

void Shell::printHelp() const {
    // print out 'help' for the user
    // in a format so it's easy to read
    for (auto it : commands) {
        std::cout << std::left << std::setw(15) << std::setfill(' ') << it.second.shortcut;
        std::cout << std::left << std::setw(15) << std::setfill(' ') << it.second.desc;
        std::cout << "\n";
    }
}

bool Shell::executeCommand(std::string input) {
    std::string unit;
    size_t size;

    // split the input line up by ' ' and get
    // the appropriate command enumeration
    std::vector<std::string> tokens = split(input, ' ');
    if (tokens.empty())
        return false;
    CMD cmd = getCommand(tokens);

    switch (cmd) {
        case INVALID:
        USER_ALERT("INVALID COMMAND");
            break;
        case UNKNOWN:
        USER_ALERT("UNKNOWN COMMAND");
            break;
        case EXIT:
            return true;
        case HELP:
            printHelp();
            break;
        case LS:
            //ls of the current directory ('ls')
            if (tokens.size() == 1)
                fileSystem->ls("");
            //ls of a particular directory ('ls /Documents/')
            else fileSystem->ls(tokens[1]);
            break;
        case CP:
            fileSystem->cp(tokens[1], tokens[2]);
            break;
        case MV:
            fileSystem->mv(tokens[1], tokens[2]);
            break;
        case RM:
            fileSystem->rm(tokens[1]);
            break;
        case MKDIR:
            fileSystem->mkdir(tokens[1]);
            break;
        case RMDIR:
            fileSystem->rmdir(tokens[1]);
            break;
        case CD:
            fileSystem->cd(tokens[1]);
            break;
        case CAT:
            fileSystem->cat(tokens[1]);
            break;
        case PWD:
            fileSystem->pwd();
            break;
        case INCP:
            // import to a specific directory e.g. 'incp file.txt /Documents'
            if (tokens.size() == 3)
                fileSystem->incpy(tokens[1], tokens[2]);
            // import to the current directory 'incp file.txt'
            else fileSystem->incpy(tokens[1]);
            break;
        case OUTCP:
            fileSystem->outcpy(tokens[1], tokens[2]);
            break;
        case INFO:
            fileSystem->info(tokens[1]);
            break;
        case LOAD:
            loadFileToExecute(tokens[1]);
            break;
        case FORMAT:
            // check if there's a unit in the line
            // such as format 500MB or format 1GB
            if (tokens[1].length() > 2)
                unit = tokens[1].substr(tokens[1].length() - 2, tokens[1].length());
            else unit = "";

            // if there is a unit - convert it into bytes
            if (unit != "") {
                size = std::stoi(tokens[1].substr(0, tokens[1].length() - 2));
                if (unit == Disk::GB)
                    size *= 1e9;
                else if (unit == Disk::MB)
                    size *= 1e6;
                else if (unit == Disk::KB)
                    size *= 1e3;
            }
            else size = std::stoi(tokens[1]);
            fileSystem->format(size);
            break;
        case SLINK:
            fileSystem->slink(tokens[1], tokens[2]);
            break;
    }
    return false;
}

std::vector<std::string> Shell::split(const std::string &str, char separator) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (getline(ss, token, separator))
        if (token != "")
            tokens.emplace_back(token);
    return tokens;
}

void Shell::loadFileToExecute(std::string path) {
    // open the file, read off it line by line
    // and used every line as a command to execute
    std::ifstream file(path);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            std::cout << line << "\n";
            executeCommand(line);
        }
        file.close();
        USER_ALERT("OK");
    }
    else {
        USER_ALERT("FILE NOT FOUND");
    }
}

Shell::CMD Shell::getCommand(std::vector<std::string> tokens) {
    if (tokens.empty())
        return UNKNOWN;
    auto it = commands.find(tokens[0]);
    if (it == commands.end())
        return UNKNOWN;
    // call the appropriate function to validate the line
    if (it->second.validation(tokens) == true)
        return it->second.cmd;
    return INVALID;
}

bool validCP(const std::vector<std::string>& tokens) {
    return tokens.size() == 3;
}

bool validMV(const std::vector<std::string>& tokens) {
    return tokens.size() == 3;
}

bool validRM(const std::vector<std::string>& tokens) {
    return tokens.size() == 2;
}

bool validMKDIR(const std::vector<std::string>& tokens) {
    return tokens.size() == 2;
}

bool validRMDIR(const std::vector<std::string>& tokens) {
    return tokens.size() == 2;
}

bool validLS(const std::vector<std::string>& tokens) {
    return tokens.size() == 1 || tokens.size() == 2;
}

bool validCAT(const std::vector<std::string>& tokens) {
    return tokens.size() == 2;
}

bool validPWD(const std::vector<std::string>& tokens) {
    return tokens.size() == 1;
}

bool validINFO(const std::vector<std::string>& tokens) {
    return tokens.size() == 2;
}

bool validINCP(const std::vector<std::string>& tokens) {
    return tokens.size() == 3 || tokens.size() == 2;
}

bool validOUTCP(const std::vector<std::string>& tokens) {
    return tokens.size() == 3;
}

bool validLOAD(const std::vector<std::string>& tokens) {
    return tokens.size() == 2;
}

bool validFORMAT(const std::vector<std::string>& tokens) {
    if (tokens.size() != 2)
        return false;

    // check if there's a unit in the line
    // such as format 500MB or format 1GB
    std::string unit;
    if (tokens[1].length() > 2)
        unit = tokens[1].substr(tokens[1].length() - 2, tokens[1].length());
    else unit = "";

    // if there is a unit - try to convert it into bytes
    // it may be too big to fit into size_t or may not contain digits only
    // this also cause a warning when compiling the whole application
    // it's okay though because the variable size is not supposed to be used
    // any further other than just testing of the size of the file
    if (unit == Disk::GB || unit == Disk::MB || unit == Disk::KB) {
        std::string value = tokens[1].substr(0, tokens[1].length() - 2);
        if (containsOnlyDigits(value) == false)
            return false;
        try {
            size_t size = std::stoi(value);
            if (unit == Disk::GB)
                size *= 1e9;
            else if (unit == Disk::MB)
                size *= 1e6;
            else if (unit == Disk::KB)
                size *= 1e3;
            return true;
        }
        catch (std::invalid_argument& e){
            return false;
        }
        catch (std::out_of_range& e) {
            return false;
        }
        catch (...) {
            return false;
        }
    }
    else {
        // check if there are digits only in the string
        // and try to convert it into bytes
        // this causes a warning when compiling but it's okay
        // as explained above
        if (containsOnlyDigits(tokens[1]) == false)
            return false;
        try {
            size_t size = std::stoi(tokens[1]);
            return true;
        }
        catch (std::invalid_argument& e){
            return false;
        }
        catch (std::out_of_range& e) {
            return false;
        }
        catch (...) {
            return false;
        }
    }
}

bool validEXIT(const std::vector<std::string>& tokens) {
    return tokens.size() == 1;
}

bool validHELP(const std::vector<std::string>& tokens) {
    return tokens.size() == 1;
}

bool validCD(const std::vector<std::string>& tokens) {
    return tokens.size() == 2;
}

bool validSLINK(const std::vector<std::string>& tokens) {
    return tokens.size() == 3;
}

bool containsOnlyDigits(std::string str) {
    for (char c : str)
        if (c < '0' || c > '9')
            return false;
    return true;
}