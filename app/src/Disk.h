#ifndef DISK_H
#define DISK_H

#include <iostream>
#include <cstdlib>
#include <memory>
#include <vector>
#include <cstring>
#include <iomanip>
#include <stack>
#include <unistd.h>

#include "Setup.h"
#include "Logger.h"

#define UNUSED(x) (void)(x)

/// \author A127B0362P silhavyj
///
/// This class represents a physical disk of the virtual
/// file system and provides functionality associated with
/// it. This class is at the very bottom of the hierarchy
/// of the project.
class Disk {
public:
    /// a GB unit (1e9 bytes) used when formatting the disk
    static const std::string GB;
    /// a MB unit (1e6 bytes) used when formatting the disk
    static const std::string MB;
    /// a KB unit (1e3 bytes) used when formatting the disk
    static const std::string KB;

    /// a 'NULL' pointer used when looking for a free cluster
    const int32_t NULL_POINTER = -1;
    /// id of the i-node holding the information about the root directory
    const int32_t ROOT_INODE_ID = 0;

public:
    /// Superblock of the file system holding all the
    /// necessary information about the system. The overall size
    /// of the superblock is 284B.
    struct SuperBlock_t {
        char signature[SIGNATURE_LEN];           ///< signature of the owner of the file system
        char volumeDescriptor[VOLUME_DESC_LEN];  ///< short description of the file system
        int32_t diskSize;         ///< disk size (B)
        int32_t clusterSize;      ///< size of a cluster (B)
        int32_t clusterCount;     ///< the total number of clusters in the file system
        int32_t bitmapStartAddr;  ///< start address of the bitmap
        int32_t iNodeStartAddr;   ///< start address of the i-nodes
        int32_t dataStartAddr;    ///< start address of the clusters
    };

    /// I-node structure holding all the information
    /// about a folder/file in the file system.
    /// The overall size of an i-node is 44B.
    struct INode_t {
        int32_t nodeId;       ///< i-node id (0,1,...,n)
        int32_t parentId;     ///< id of the parent of the i-node
        bool isFree;          ///< flag if the i-node is free
        bool isDirectory;     ///< flag if the i-node is a directory
        bool isSymbolicLink;  ///< flag if the i-node is a symbolic link
        int32_t size;         ///< total size of the i-node
        int32_t direct[NUM_OF_DIRECT_POINTERS];     ///< direct pointers to the clusters making up the file/folder
        int32_t indirect[NUM_OF_INDIRECT_POINTERS]; ///< indirect pointers to the clusters making up the file/folder
    };

    /// DirectoryItem structure holding information
    /// about an item in a folder, which can be either
    /// another folder or a file. The overall size
    /// of a DirectoryItem is 16B.
    struct DirectoryItem_t {
        int32_t iNode;                ///< i-node id (0,1,...,n)
        char itemName[FILE_NAME_LEN]; ///< name of the item
    };

    /// DirectoryItems structure holding all
    /// items within the directory (filer, other folders)
    /// The whole size of the structure is
    /// sizeof(size_t) + count * sizeof(DirectoryItem_t) =
    /// (8 + count * 16)B
    struct DirectoryItems_t {
        size_t count;                   ///< number of items in the folder
        DirectoryItem_t *items = NULL;  ///< directory items themselves

        /// Constructor - creates an instance of the structure
        DirectoryItems_t() {}

        /// Constructor - creates an instance of the structure
        ///
        /// This type of constructor is used, for example, when
        /// creating the root directory.
        ///
        /// \param iNodeId id of the i-node
        /// \param iNodeParentId id of the parent of the i-node
        DirectoryItems_t(int32_t iNodeId, int32_t iNodeParentId);

        /// Destructor of the structure.
        ///
        /// It deletes all the items in the
        /// folder from the memory.
        ~DirectoryItems_t();
    };

private:
    int CLUSTER_COUNT;               ///< the total number of clusters in the file system
    FILE *diskFile = NULL;           ///< reference to the storage of the file system
    SuperBlock_t *superBlock = NULL; ///< reference to the superblock of the class
    bool *bitmap = NULL;             ///< the bitmap of the file system
    std::string diskFileName;        ///< the name of the storage (file) of the file system
    INode_t iNodes[INODES_COUNT];    ///< the i-nodes of the file system
    INode_t *currentINode = NULL;    ///< reference to the current i-node (current location)

public:
    /// Destructor of the class
    ///
    /// It deletes all the pre-allocated blocks of memory
    /// and closes the file descriptor of the storage (file)
    /// of the file system.
    ~Disk();

    /// Constructor of the class - creates an instance of the class
    ///
    /// \param diskFileName name of the storage file of the file system
    Disk(std::string diskFileName);

    /// Copy constructor of the class
    ///
    /// It was manually deleted since there is no need to use
    /// it within this project.
    Disk(const Disk &) = delete;

    /// Assignment operator of the class.
    ///
    /// It was manually deleted since there is no need to use
    /// it within this project.
    void operator=(Disk const &) = delete;

    /// Normalizes the string to 12B as it's defined in Setup.h
    ///
    /// If the string given as a parameter is a size of
    /// more than 12B, the beginning of the string will be cut off
    /// so the rest of it is exactly 12B (including '\0').
    ///
    /// \param name we want to normalize to 12B
    /// \return normalized string
    std::string normalizeName(std::string name);

    /// Formats the disk with a new size given as a parameter in bytes.
    ///
    /// When the user enters the commands, they can also add a unit as they
    /// enter the number. For example, 600MB, 5GB, and so on.
    ///
    /// \param diskSize new size of the disk
    void format(size_t diskSize);

    /// Return an i-node from the path given as a parameter.
    ///
    /// If the path is invalid (the target is not found), value
    /// NULL will be returned. The target not can be both a directory
    /// or a file.
    ///
    /// \param path to the target i-node
    /// \return the target i-node from the path
    INode_t *getINodeFromPath(std::string path);

    /// Removes a file from the file system. 
    /// 
    /// This method deletes the file (i-node) given as a parameter.
    ///
    /// \param iNode file that is going to be deleted
    void removeFile(INode_t *iNode);

    /// Creates a new folder in the directory (path) given as a parameter.
    ///
    /// \param destinationINode i-node of the destination folder
    /// \param folderName name of the folder that is going to be created
    void addNewFolder(INode_t *destinationINode, std::string folderName);

    /// Creates a new folder in the current directory with a name given as a parameter
    ///
    /// \param folderName name of the folder that is going to be created
    void addNewFolder(std::string folderName);

    /// Removes a directory (i-node) given as a parameter
    ///
    /// \param iNode i-node of the target folder that is going to be deleted
    void removeDirectory(INode_t *iNode);

    /// Changes the current location (path)
    ///
    /// \param path new current location
    void cd(std::string path);

    /// Prints out the content of the current directory.
    void printCurrentDirectoryItems();

    /// Prints out directory items (content of a directory) given as a parameter
    ///
    /// \param directoryItems directory items that are going to be printed out
    void printDirectoryItems(const DirectoryItems_t *directoryItems);

    /// Returns directory items from the i-node given as a parameter
    ///
    /// This method is used when modidying its content
    /// - adding/deleting files/folders, renaming them, etc.
    ///
    /// \param iNode i-node of the folder containing the items we want to get
    /// \return directory items of the folder (i-node) given as a parameter.
    DirectoryItems_t *getDirectoryItemsFromINode(INode_t *iNode);

    /// Returns the current path as a string
    ///
    /// The format of the path is absolute, meaning it goes all the way down
    /// from the root directory down to the target (current) directory
    ///
    /// \return current location as an absolute path
    std::string getCurrentPath();

    /// Prints out the content of the file given as a parameter.
    ///
    /// As there might be symbolic links in the file system as well, we 
    /// need to specify if we want to print out the name of a symbolic link
    /// in a specific format (e.g. myLink -> /Doc/data.pdf) or the contnet 
    /// of the file the symbolic link points at. To tackle this, there is
    /// another parameter used to distingush these two approaches. An example
    /// when we want to print out only the "name" of a symbolic link is
    /// when printing out a content of a directory (#printDirectoryItems).
    /// The other case might be when we want to print out the file content using
    /// command cat.
    ///
    /// \param iNode iNode of the file we want to print out
    /// \param includeSlinks true/false whether we want to print out symbolic links as well.
    void printFileContent(INode_t *iNode, bool includeSlinks);

    /// Imports the file given as a paramter into the virtual file system
    ///
    /// This method reads the content of the file block by block, stores it into
    /// clusters, and attaches the clusters to an i-node holding all the information
    /// about the file. 
    ///
    /// \param destinationINode i-node of the folder we are importing the file into within the file system
    /// \param sourceFile the file descriptior of the source file located on the HDD
    /// \param fileName the name of the file in the virtual file system
    void incpyFile(INode_t *destinationINode, FILE *sourceFile, std::string fileName);

    /// Exports the file given as a paramter (i-node) from the virtual file system onto the HDD
    ///
    /// First, it collects all the clusters the file content is stored in and then,
    /// it will copy the content out into the destination file on the HDD.
    ///
    /// \param sourceINode i-node of the source file within the file system
    /// \param destinationFile the file desctiptor of the destination file on the HDD
    void outcpyFile(INode_t *sourceINode, FILE *destinationFile);

    /// Copies the file give as a parameter (i-node) to a different directory
    ///
    /// It does not have to be necessary a different directory if the name if the file differs.
    /// If the user tries to copy the file into the same directory with the same name as 
    /// the original file, an error message will be thrown.
    ///
    /// \param fileINode i-node of the source file (that is going to be copied)
    /// \param destinationINode i-node of the destination directory
    /// \param fileName name of copied copied file
    void copyFileToADifferentDirectory(INode_t *fileINode, INode_t *destinationINode, std::string fileName);

    /// Moves the file given as a parameter to a different directory.
    ///
    /// This method can be also used for renaming files within the same directory.
    ///
    /// \param fileINode i-node of the source file (that is going to be moved)
    /// \param destinationINode i-node of the destination directory the file is being moved into
    /// \param fileName name of the moved file
    void moveFileToADifferentDir(INode_t *fileINode, INode_t *destinationINode, std::string fileName);

    /// Prints out all information about the i-node given as a parameter
    ///
    /// It prints out all the information such as id, parent's id, number of clusters
    /// the i-node hold pointer to, size, whether it's a directory or a file, and so on.
    ///
    /// \param iNode i-node we want to print out info about
    void printInfoAboutINode(INode_t *iNode);

    /// Creates a symbolic link pointing at the file given as a parameter.
    ///
    /// The symbolic link is another i-node holding a path the the original
    /// file and is treated accordingly. For example, if the user runs command
    /// cat, it will print out the content of the original file. The same approach
    /// is used when exporting the file onto the HDD.
    ///
    /// \param fileINode i-node of the source file we want to create a symbolic link to
    /// \param slinkName name of the symbolic link that is being created
    void createSymbolicLink(INode_t *fileINode, std::string slinkName);

private:
    /// Creates a new file system
    ///
    /// It creates sequentially all data structures representing
    /// the file system as a whole - superblock, bitmap, inodes, and so on. 
    ///
    /// \param diskSize size of the storage (file) in bytes
    void initNewFileSystem(size_t diskSize);

    /// Initializes a new superblock of the file system
    ///
    /// \param diskSize size of the storage (file) in bytes
    void initNewSuperBlock(size_t diskSize);

    /// Re-initializes all i-nodes in the file system
    void initINodes();

    /// Re-initializes the bitmap of the file system
    void initBitmap();

    /// Saves the whole file system on the disk
    ///
    /// This method is called when the user formats the file system
    /// because all the data structure need to be stored at the same time.
    void saveFileSystemOnDisk();

    /// Stores the superblock in the file (storage)
    void saveSuperblokOnDisk();

    /// Stores the bitmap in the file (storage)
    void saveBitmapOnDisk();

     /// Stores the i-nodes in the file (storage)
    void saveINodesOnDisk();

    /// Stores the root directory on the disk.
    /// 
    /// This part is really crucial as the file system
    /// need to know where to look for the entry point
    /// the whole hierarchy
    void saveRootDirectoryOnDisk();

    /// Stores directory (directory items) on the disk
    ///
    /// \param iNode i-node of the directory
    /// \param directoryItems directory items (content) themeselves
    void saveDirectoryItemsOnDisk(INode_t *iNode, DirectoryItems_t *directoryItems);

    /// Loads the whole system from the disk
    ///
    /// This method is called when the program starts.
    void loadFileSystemFromDisk();

    /// Loads the superblock from the disk
    void loadSuperBlockFromDisk();

    /// Loads the bitmap from the disk
    void loadBitmapFromDisk();

    /// Loads all the i-nodes from the disk
    void loadINodesFromDisk();

    /// Prints out the whole file system.
    ///
    /// It prints out the superblock as well as the bitmap
    /// and the inodes. This method is not currently used
    /// anywhere whithin the project since its output is
    /// quite big but it could be used if the user wants to
    /// see what the file system looks like
    void printFileSystem();

    /// Prints out the superblock of the file system
    void printSuperblock() const;

    /// Prints out the bitmap of the file system
    void printBitmap() const;

    /// Prints out all the i-nodes of the file system
    void printINodes() const;

    /// Prints out an i-node in (all the information about it)
    ///
    /// \param iNode i-node that is going to be printed out
    void printINode(const INode_t *iNode) const;

    /// Prints out a directory item (file/folder)
    ///
    /// \param directoryItem directory item that is going to be printed out
    void printDirectoryItem(const DirectoryItem_t *directoryItem);

    /// Returns the number of clusters based on the size give as a parameter
    ///
    /// If the size is greater than zero, the n number of clusters needed is
    /// at least one. Otherwise it returns the precise number of cluster that
    /// is needed to store the file/folder based on the size of one cluster.
    ///
    /// \param size of the file/folder we want to store into clusters
    /// \return number of clusters needed to store the file/folder
    inline int32_t getNumberOfClustersNeeded(int32_t size) const;

    /// Returns the date offset based on the index given as a parameter
    ///
    /// The date offset is calculated as the start address of clusters
    /// (data) + index * size of one cluster. This method helps you not 
    /// to use the same long expression over and over again.
    ///
    /// \param index index of the cluster we want to move to within the file (storage)
    /// \return the start address of the cluster we want to move to
    inline int32_t dataOffset(int32_t index) const;

    /// Splits up the string given as a parameter by the separator character
    ///
    /// This method is used to split a string into individual tokens when
    /// analyzing a path to a target file/folder ('/data/doc.png').
    ///
    /// \param str string we want to split up
    /// \param separator character by which we want to split the string up
    /// \return a vector of split up tokens
    std::vector<std::string> split(const std::string& str, char separator);

    /// Returns a free i-node
    ///
    /// This method is used when creating a new file/folder
    /// or when a file is being moved to a different directory.
    ///
    /// \return a reference to a free i-node. If all the i-nodes are
    /// occupied at the moment, it will return NULL
    INode_t *getFreeINode();


    /// Returns a free an index of a free cluster
    ///
    /// This method is widely used when importing a new file into
    /// the virtual file system, as well as when copying a file into
    /// a different directory.
    ///
    /// \return an index of a free cluster. If there are no free clusters
    /// in the file system, it will return #NULL_POINTER
    int32_t getFreeCluster();

    /// Initializes a new root directory
    ///
    /// This method is used when the user formats the file system
    /// with a new size and all the data structures need to be re-initialized.
    void initializeRootINode();

    /// Assignes direct clusters to an i-node.
    ///
    /// \param iNode i-node we want to assign direct clusters to
    /// \return true if everything goes well. False, if there is not enough
    /// clusters in the file system
    bool addDirectClustersToINode(INode_t *iNode);

    /// Finds out if there is at least n free clusters in te file system
    ///
    /// This method is used when importing a file into the file system
    /// as well as copying a file within the system.
    ///
    /// \param n number of cluster we requite to be store a file/folder
    /// \return true, if there is at least n clusters free. Otherwise, false.
    bool isThereAtLeastNFreeClusters(int32_t n);

    /// Finds out whether or not there is a file/folder in the directory given as a parameter with particular the name
    ///
    /// \param directoryItems directory items
    /// \param name name that we want to find out whether or not is already in the folder
    /// \return true, if the name exists withing the folder. Otherwise, false.
    bool existsInDirectory(DirectoryItems_t *directoryItems, std::string name);

    /// Adds the i-node given as a parameter to the particulat directory
    ///
    /// This method is used when adding a new file/folder into the directory
    /// given as a parameter. 
    ///
    /// \param directoryItems directory items of the target directory
    /// \param directoryINode i-node of the target directory
    /// \param newINode i-node (file/folder) we are going to add into the directory
    /// \param name of the (file/folder) we are going to add into the directory
    void addINodeToDirectory(DirectoryItems_t *directoryItems, INode_t *directoryINode, INode_t *newINode, std::string name);

    /// Returns all the clusters of the i-node given as a parameter
    ///
    /// This method is used when printing out/exporting a file
    /// located in the virtual file system. It returns all the clusters that the
    /// content of the file is stored in.
    ///
    /// \param iNode i-node of the file we want to get all clusters of
    /// \return a vector of all the clusters of the i-node
    std::vector<std::int32_t> getAllClustersOfINode(INode_t *iNode);

    /// Attach clusters containing a content of a file to the i-node given as a parameter
    ///
    /// \param iNode i-node we want to attach the clusters to
    /// \param clusters all the clusters we want  to attach to the i-node
    /// \return false, if there is not enough free clusters in the file system. Otherwise, true. 
    bool attachClustersToINode(INode_t *iNode, std::vector<int32_t> clusters);

    /// Removes the i-node given as a parameter from its parent.
    ///
    /// It means that the i-node will be removed from the directory
    /// that it is located in.
    ///
    /// \param iNode i-node that is going to be removed from the parent directory
    void removeINodeFromParent(INode_t *iNode);

    /// Removes the i-node given as a parameter from the file system.
    ///
    /// The i-node can be both a directory or a folder, and the method
    /// is called when the user wants to delete either of these entities
    /// using methods #removeDirectory and #removeFile
    ///
    /// \param iNode that is going to be deleted
    void removeINode(INode_t *iNode);
    
    /// Returns an i-node from the path given as a parameter
    ///
    /// The user can type either a relative or absolute path
    /// leading to the target i-node.
    ///
    /// \param iNode current location (directory)
    /// \param path the user typed down as they want to get to the targer file/folder
    /// \param relative whether it is a relative or absolute path
    /// \return target i-node if found. Otherwise, NULL.
    INode_t *getINodeFromPath(INode_t *iNode, std::string path, bool relative);

    /// Returns an absolute path of the i-node given as a parameter
    ///
    /// This path always starts from the root directory. Therefore,
    /// the format looks like '/Data/Monday/'.
    ///
    /// \param iNode i-node which we went to know an absolute path of
    /// \return an absolute path of the i-node given as a parameter
    std::string getPath(INode_t *iNode);

    /// Returns the content of the slink given as a parameter (path to the file the slink points at)
    ///
    /// \param iNode symbolic link to a file
    /// \return the content of the symbolic like (an absolute path to a file)
    std::string getPathFromSLink(INode_t *iNode);
};

#endif
