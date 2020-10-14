#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <iostream>
#include <memory>
#include <string>
#include <fstream>

#include "Disk.h"
#include "Logger.h"

/// \author A127B0362P silhavyj
///
/// This class provides all the features of the file system
/// as it is described in the document available on courseware.
/// The class directly interacts with class #Disk, which represents
/// a virtual disk of the file system
class FileSystem {
private:
    /// an instance of class #Disk, which represents
    /// a virtual disk of the file system
    Disk *disk = NULL;

private:
    /// Returns a destination file name from the path given as a parameter.
    ///
    /// As the user enters a path, there are several ways of what they might
    /// want to do. For example, they can specify the target file name directly
    /// e.g. 'file.txt' or 'doc/file.txt'. In either of these cases, the method
    /// will return 'file.txt'. Another case is when the user enters only a target
    /// directory. For example, 'incp file.txt doc/zos/'. In this case, the target
    /// file name is the same as the original file - 'file.txt'. This method is used
    /// when copying/moving files within the file system as well as
    /// importing/exporting files.
    ///
    /// \param source source file e.g. /doc/source.txt
    /// \param destination which can be either a file or only a directory
    /// \return the destination file name
    std::string getDestinationFileName(std::string source, std::string destination);

    /// Returns a destination directory form the path given as a parameter.
    ///
    /// As the user enters a path, there are several ways of what they might
    /// want to do. For example, if the specifies only a target file, the
    /// method will return the current directory as it assumes the user wants
    /// to work within the current location. In all other cases, the method
    /// will cut off the target file and return the remaining string as a path
    /// to the file. For example, 'cp dat/file.txt /movies/file2.txt' will return
    /// '/movies/'. This method is used when copying/moving withing the file system
    /// as well as importing/exporting files.
    ///
    /// \param destination destination path e.g. 'doc/zos/file.txt'
    /// \return the destination folder e.g. 'doc/zos/'
    std::string getDestinationDirectoryPath(std::string destination);

    /// Returns the name of the source file as the user enters a path
    ///
    /// For example, if the path is '/doc/home/data.pdf', it will
    /// return data.pdf. This method is used by method #getDestinationFileName.
    ///
    /// \param source source path
    /// \return source file (the end of the path)
    std::string getSourceFileName(std::string source);

public:
    /// Constructor of the class - creates an instance of it.
    ///
    /// \param diskFileName name of the file system (storage e.g. 'data.dat')
    FileSystem(std::string diskFileName);

    /// Destructor of the class.
    ///
    /// Is deletes all dynamically allocated structures
    /// from the memory.
    ~FileSystem();

    /// Copy constructor of the class.
    ///
    /// The was manually disabled since there is no need
    /// to use is method within this project.
    FileSystem(const FileSystem &) = delete;

    /// Assignment operator of the class.
    ///
    /// The method was manually disabled since there is no need
    /// to use it method within this project.
    void operator=(FileSystem const &) = delete;

    /// Returns the current location as an absolute path.
    /// \return current path
    std::string getCurrentPath() const;

    /// Removes a file from the file system.
    /// ### Example
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// rm img01.png
    /// rm ../../doc/img01.png
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// \param path (absolute/relative) to the target file
    void rm(std::string path);

    /// Creates a new directory in the location given as a parameter.
    /// ### Example
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// mkdir test
    /// mkdir test/ZOS/test01
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// \param path (absolute/relative) to the target directory including the name
    void mkdir(std::string path);

    /// Removes an empty directory from the file system.
    /// ### Example
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// rmdir test
    /// rmdir test/ZOS/test01
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// \param path (absolute/relative) to the target directory
    void rmdir(std::string path);

    /// Changes the current location within the file system.
    /// ### Example
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// cd test
    /// cd ..
    /// cd test/ZOS
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// \param path to the new current location
    void cd(std::string path);

    /// Prints out the content of the folder.
    /// ### Example
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// ls
    /// ls ../ZOS
    /// ls ..
    /// ls test/ZOS
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// \param path (absolute/relative) to the target directory
    void ls(std::string path);

    /// Prints out the current location as an absolute path
    /// ### Example
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// pwd
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    void pwd();

    /// Prints out the content of the file
    /// ### Example
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// cat data.txt
    /// cat ../items.csv
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// \param path (absolute/relative) to the target file
    void cat(std::string path);

    /// Imports the file into the virtual file system into the current location.
    /// ### Example
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// incp test_files/data.txt
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// \param source path (absolute/relative) to the source file on the HDD
    void incpy(std::string source);

    /// Imports the file into the virtual file system into the location given as a parameter.
    /// ### Example
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// incp test_files/data.txt Documents/data_copied.txt
    /// incp test_files/data.txt Documents/
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// \param source path (absolute/relative) to the source file on the HDD
    /// \param destination target destination within the file system
    void incpy(std::string source, std::string destination);

    /// Exports the file from the file system onto the HDD.
    /// ### Example
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// outcp test_files/data.txt data_exported.txt
    /// outcp data.txt Documents/data_exported.txt
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// \param source path (absolute/relative) to the source file within the file system
    /// \param destination path (absolute/relative) to the target file on the HDD
    void outcpy(std::string source, std::string destination);

    /// Formats the disk with a new size given as a parameter (B).
    /// ### Example
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// format 500MB
    /// format 5GB
    /// format 50MB
    /// format 20KB
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// \param size new size of the disk (B)
    void format(size_t size);

    /// Copies the file within the file system.
    /// ### Example
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// cp data1.txt data2.txt
    /// cp data1.txt Documents/data2.txt
    /// cp ../data1.txt data2.txt
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// \param source path (absolute/relative) to the source file within the file system
    /// \param destination path (absolute/relative) to the target file within the file system
    void cp(std::string source, std::string destination);

    /// Movies the file within the file system. It can be also used for renaming files.
    /// ### Example
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// mv data1.txt data2.txt
    /// mv data1.txt Documents/data2.txt
    /// mv ../data1.txt data2.txt
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// \param source path (absolute/relative) to the source file within the file system
    /// \param destination path (absolute/relative) to the target file within the file system
    void mv(std::string source, std::string destination);

    /// Prints out info about the i-node (clusters, size, index, ...)
    /// ### Example
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// info data1.txt
    /// info .
    /// info ../Documents
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// \param path (absolute/relative) to the target file/folder within the file system
    void info(std::string path);

    /// Creates a symbolic link pointing at the target file
    /// ### Example
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// slink data.txt link1
    /// slink ../data.txt link2
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// \param file target file the symbolic link will be pointing at
    /// \param name of the symbolic link
    void slink(std::string file, std::string name);
};

#endif