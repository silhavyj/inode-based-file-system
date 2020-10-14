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

    /// a 'NULL' pinter used when looking for a free i-node
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
        int32_t direct[NUM_OF_DIRECT_POINTERS];       ///< direct pointers to the clusters making up the file/folder
        int32_t indirect[NUM_OF_INDIRECT_POINTERS];   ///< indirect pointers to the clusters making up the file/folder
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


    void removeFile(INode_t *iNode);
    void addNewFolder(INode_t *destinationINode, std::string folderName);
    void addNewFolder(std::string folderName);
    void removeDirectory(INode_t *iNode);
    void cd(std::string path);
    void printCurrentDirectoryItems();
    void printDirectoryItems(const DirectoryItems_t *directoryItems);
    DirectoryItems_t *getDirectoryItemsFromINode(INode_t *iNode);
    std::string getCurrentPath();
    void printFileContent(INode_t *iNode, bool includeSlinks);
    void incpyFile(INode_t *destinationINode, FILE *sourceFile, std::string fileName);
    void outcpyFile(INode_t *sourceINode, FILE *destinationFile);
    void copyFileToADifferentDirectory(INode_t *fileINode, INode_t *destinationINode, std::string fileName);
    void moveFileToADifferentDir(INode_t *fileINode, INode_t *destinationINode, std::string fileName);
    void printInfoAboutINode(INode_t *iNode);
    void createSymbolicLink(INode_t *fileINode, std::string slinkName);

private:
    void initNewFileSystem(size_t diskSize);
    void initNewSuperBlock(size_t diskSize);
    void initINodes();
    void initBitmap();

    void saveFileSystemOnDisk();
    void saveSuperblokOnDisk();
    void saveBitmapOnDisk();
    void saveINodesOnDisk();
    void saveRootDirectoryOnDisk();
    void saveDirectoryItemsOnDisk(INode_t *iNode, DirectoryItems_t *directoryItems);

    void loadFileSystemFromDisk();
    void loadSuperBlockFromDisk();
    void loadBitmapFromDisk();
    void loadINodesFromDisk();

    void printFileSystem();
    void printSuperblock() const;
    void printBitmap() const;
    void printINodes() const;
    void printINode(const INode_t *iNode) const;
    void printDirectoryItem(const DirectoryItem_t *directoryItem);

    inline int32_t getNumberOfClustersNeeded(int32_t size) const;
    inline int32_t dataOffset(int32_t index) const;
    std::vector<std::string> split(const std::string& str, char separator);

    INode_t *getFreeINode();
    int32_t getFreeCluster();
    void initializeRootINode();
    bool addDirectClustersToINode(INode_t *iNode);
    bool isThereAtLeastNFreeClusters(int32_t n);
    bool existsInDirectory(DirectoryItems_t *directoryItems, std::string name);
    void addINodeToDirectory(DirectoryItems_t *directoryItems, INode_t *directoryINode, INode_t *newINode, std::string name);
    std::vector<std::int32_t> getAllClustersOfINode(INode_t *iNode);
    bool attachClustersToINode(INode_t *iNode, std::vector<int32_t> clusters);
    void removeINodeFromParent(INode_t *iNode);

    /// Removes the i-node given as a parameter from the file system.
    ///
    /// The i-node can be both a directory or a folder, and the method
    /// is called when the user wants to delete either of these entities
    /// using methods #removeDirectory and #removeFile
    ///
    /// \param iNode that is going to be deleted
    void removeINode(INode_t *iNode);
    
    INode_t *getINodeFromPath(INode_t *iNode, std::string path, bool relative);
    std::string getPath(INode_t *iNode);
    std::string getPathFromSLink(INode_t *iNode);
};

#endif