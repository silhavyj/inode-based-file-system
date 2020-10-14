#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <sstream>

#define ENABLE_LOG_ERROR   ///< enable/disable error logging
#define ENABLE_LOG_WARNING ///< enable/disable warning logging
//#define ENABLE_LOG_INFO  ///< enable/disable info logging

/// Prints out an error message appropriately formatted.
///
/// If the error logging is enabled, it will call
/// method log of class #Logger in order to print the error message out
/// in the terminal as a log of what just happened.
/// Otherwise, this calling will be simply ignored.
#ifdef ENABLE_LOG_ERROR
# define LOG_ERR(msg) \
  FORMAT_LINE_NUMBER(__LINE__) \
  Logger::getInstance()->log(Logger::ERROR, (msg))
#else
# define LOG_ERR(msg) do { } while(0)
#endif

/// Prints out a warning message appropriately formatted.
///
/// If the warning logging is enabled, it will call
/// method log of class #Logger in order to print the error message out
/// in the terminal as a log of what just happened.
/// Otherwise, this calling will be simply ignored.
#ifdef ENABLE_LOG_WARNING
# define LOG_WARNING(msg) \
  FORMAT_LINE_NUMBER(__LINE__) \
  Logger::getInstance()->log(Logger::WARNING, (msg))
#else
# define LOG_WARNING(msg) do { } while(0)
#endif

/// Prints out an info message appropriately formatted.
///
/// If the info logging is enabled, it will call
/// method log of class #Logger in order to print the error message out
/// in the terminal as a log of what just happened.
/// Otherwise, this calling will be simply ignored.
#ifdef ENABLE_LOG_INFO
# define LOG_INFO(msg) \
  FORMAT_LINE_NUMBER(__LINE__) \
  Logger::getInstance()->log(Logger::INFO, (msg))
#else
# define LOG_INFO(msg) do { } while(0)
#endif

/// Prints out a message for the user.
///
/// All the messages are specified in the document
/// available on coursware. These are, for example,
/// 'OK', 'FILE NOT FOUND' etc. They were all put in
/// this macro so the output could be formatted later
/// on if required.
#define USER_ALERT(msg) \
std::cout << msg << "\n"

/// Formats the number given as a parameter into a three-digit format
///
/// This macro is used by macros #LOG_ERR, #LOG_WARNING,
/// and #LOG_INFO to format the number of the line
/// given as a parameter.
#define FORMAT_LINE_NUMBER(x) \
std::cout << "["; \
if (x < 10) std::cout << "000" << x; \
else if (x >= 10 && x < 100) std::cout << "00" << x; \
else if (x >= 100 && x < 1000) std::cout << "0" << x; \
else std::cout << x; \
std::cout << "]";

/// \author A127B0362P silhavyj
///
/// This class has the purpose of a logger and is used for debugging the code.
///
/// There are three levels of logging this class provides. These are ERROR, WARNING,
/// and INFO, all of which the user can either disable or enable. These logging methods
/// are spread out throughout the code so the programmer can see what exactly is
/// going on in the process of debugging. Before the application is released, all methods
/// could be disabled so the user is not annoyed with information they do not really need.
/// When the application crashes, for example, the programmer can turn them all on in order
/// to see the point where the program most likely crashed and why. In terms of implementation,
/// the class is designed as a singleton.
class Logger {
public:
    const std::string RESET   = "\033[0m";  ///< reset of the color
    const std::string BLACK   = "\033[30m"; ///< black color used when printing out into terminal
    const std::string RED     = "\033[31m"; ///< red color used when printing out into terminal
    const std::string GREEN   = "\033[32m"; ///< green color used when printing out into terminal
    const std::string YELLOW  = "\033[33m"; ///< yellow color used when printing out into terminal
    const std::string BLUE    = "\033[34m"; ///< blue color used when printing out into terminal
    const std::string MAGENTA = "\033[35m"; ///< magenta color used when printing out into terminal
    const std::string CYAN    = "\033[36m"; ///< cyan color used when printing out into terminal
    const std::string WHITE   = "\033[37m"; ///< white color used when printing out into terminal

    /// types of logging
    enum Type {
        ERROR,   ///< error (for example an i-node is NULL)
        INFO,    ///< info (for example when a method is called)
        WARNING  ///< warning (for example when a path to the file does not exist)
    };

private:
    /// the only instance of this class (singleton pattern)
    static Logger* instance;

private:
    /// Constructor of the class - creates an instance of it
    ///
    /// The constructor is private on purpose so it follows the singleton pattern
    Logger() {}

    /// Prints out a log message in the terminal
    ///
    /// The message has an appropriate color depending on
    /// the type of the message.
    ///
    /// \param type of the logging #Type
    /// \param color color of the message
    /// \param msg the message itself that is going to be printed out
    void log(std::string type, std::string color, std::string msg);

public:
    /// Copy constructor of the class - deleted since there is no need to copy an instance of this class.
    Logger(Logger &) = delete;

    /// Assignment operator - deleted since there is no need to use it within this program.
    void operator=(Logger const &) = delete;

    /// Returns the instance of the class.
    ///
    /// It also creates an instance of the class,
    /// if it has not been created yet, so it follows
    /// the singleton pattern.
    ///
    /// \return the instance of the class
    static Logger *getInstance();

    /// Assignes the appropriate color (according to the type)
    /// to the message given as a parameter and calls method
    /// log to print out the message
    ///
    /// \param type type of the message #Type
    /// \param msg message that is going to be printed out
    void log(Type type, std::string msg);

    /// Prints out all the logging types messages.
    ///
    /// The purpose of this method is to show to the user
    /// what the output looks like, so they can change it
    /// if they wish. Therefore, calling of this method is
    /// not required anywhere in the program.
    void testAllTypes();
};

#endif