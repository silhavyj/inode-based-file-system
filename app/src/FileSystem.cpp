#include "FileSystem.h"

FileSystem::FileSystem(std::string diskFileName) {
    disk = new Disk(diskFileName);
}

FileSystem::~FileSystem() {
    if (disk != NULL)
        delete disk;
}

std::string FileSystem::getCurrentPath() const {
    return disk->getCurrentPath();
}

void FileSystem::rm(std::string path) {
    Disk::INode_t *fileINode = disk->getINodeFromPath(path);
    disk->removeFile(fileINode);
}

void FileSystem::mkdir(std::string path) {
    std::string name;
    std::size_t pos = path.find_last_of('/');

    // find out whether the user specified
    // a path to the folder or just entered a name of the folder
    // mkdir A vs mkdir Documents/A
    if (path.find('/') != std::string::npos) {
        name = path.substr(pos + 1, path.length());
        name = disk->normalizeName(name);
        Disk::INode_t *folderINode = disk->getINodeFromPath(path.substr(0, pos));
        disk->addNewFolder(folderINode, name);
    }
    else {
        name = disk->normalizeName(path);
        disk->addNewFolder(name);
    }
}

void FileSystem::rmdir(std::string path) {
    Disk::INode_t *directoryINode = disk->getINodeFromPath(path);
    disk->removeDirectory(directoryINode);
}

void FileSystem::cd(std::string path) {
    disk->cd(path);
}

void FileSystem::ls(std::string path) {
    if (path.empty())
        disk->printCurrentDirectoryItems();
    else {
        Disk::INode_t *directoryINode = disk->getINodeFromPath(path);
        if (directoryINode == NULL) {
            USER_ALERT("PATH NOT FOUND");
            return;
        }
        if (directoryINode->isDirectory == false) {
            USER_ALERT("TARGET IS NOT A DIRECTORY");
            return;
        }
        Disk::DirectoryItems_t *directoryItems = disk->getDirectoryItemsFromINode(directoryINode);
        disk->printDirectoryItems(directoryItems);
        delete directoryItems;
    }
}

void FileSystem::pwd() {
    std::cout << disk->getCurrentPath() << "\n";
}

void FileSystem::cat(std::string path) {
    Disk::INode_t *fileInode = disk->getINodeFromPath(path);
    disk->printFileContent(fileInode, true);
}

void FileSystem::incpy(std::string source) {
    incpy(source, getCurrentPath());
}

void FileSystem::incpy(std::string source, std::string destination) {
    std::string fileName = getDestinationFileName(source, destination);
    fileName = disk->normalizeName(fileName);
    std::string destinationPath = getDestinationDirectoryPath(destination);

    // open the source file stored on the HDD as a binary file
    FILE *file = fopen(source.c_str(), "rb+");
    Disk::INode_t *destinationINode = disk->getINodeFromPath(destinationPath);

    disk->incpyFile(destinationINode, file, fileName);
    if (file != NULL)
        fclose(file);
}

void FileSystem::outcpy(std::string source, std::string destination) {
    // create an empty file that's going to be used
    // as a target file
    FILE *targetFile = fopen(destination.c_str(),"w");
    if (targetFile == NULL) {
        USER_ALERT("PATH NOT FOUND");
        return;
    }
    fclose(targetFile);

    // open the file again so the program
    // can write into it
    targetFile = fopen(destination.c_str(), "rb+");
    Disk::INode_t *sourceINode = disk->getINodeFromPath(source);
    disk->outcpyFile(sourceINode, targetFile);
    fclose(targetFile);
}

void FileSystem::format(size_t size) {
    disk->format(size);
}

void FileSystem::cp(std::string source, std::string destination) {
    // get the destination file and path
    std::string fileName = getDestinationFileName(source, destination);
    std::string destinationPath = getDestinationDirectoryPath(destination);
    fileName = disk->normalizeName(fileName);

    Disk::INode_t *sourceINode = disk->getINodeFromPath(source);
    Disk::INode_t *destinationINode = disk->getINodeFromPath(destinationPath);
    disk->copyFileToADifferentDirectory(sourceINode, destinationINode, fileName);
}

void FileSystem::mv(std::string source, std::string destination) {
    // get the destination file and path
    std::string fileName = getDestinationFileName(source, destination);
    std::string destinationPath = getDestinationDirectoryPath(destination);
    fileName = disk->normalizeName(fileName);

    Disk::INode_t *sourceINode = disk->getINodeFromPath(source);
    Disk::INode_t *destinationINode = disk->getINodeFromPath(destinationPath);
    disk->moveFileToADifferentDir(sourceINode, destinationINode, fileName);
}

std::string FileSystem::getSourceFileName(std::string source) {
    size_t pos = source.find_last_of("/");
    if (pos == std::string::npos)
        return source;
    return source.substr(pos + 1, source.length());
}

std::string FileSystem::getDestinationFileName(std::string source, std::string destination) {
    size_t pos = destination.find_last_of("/");
    if (pos == std::string::npos)
        return destination;
    if (pos == destination.length() - 1)
        return getSourceFileName(source);
    Disk::INode_t *destinationINode = disk->getINodeFromPath(destination);
    if (destinationINode == NULL)
        return destination.substr(pos + 1, destination.length());

    if (destinationINode->isDirectory == false)
        return destination.substr(pos + 1, destination.length());
    return getSourceFileName(source);
}

std::string FileSystem::getDestinationDirectoryPath(std::string destination) {
    size_t pos = destination.find_last_of("/");
    if (pos == std::string::npos)
        return disk->getCurrentPath();
    if (pos == destination.length() - 1)
        return destination;
    Disk::INode_t *destinationINode = disk->getINodeFromPath(destination);
    if (destinationINode == NULL || destinationINode->isDirectory == false)
        return destination.substr(0, pos);
    return destination;
}

void FileSystem::info(std::string path) {
    Disk::INode_t *iNode = disk->getINodeFromPath(path);
    disk->printInfoAboutINode(iNode);
}

void FileSystem::slink(std::string file, std::string name) {
    Disk::INode_t *iNode = disk->getINodeFromPath(file);
    disk->createSymbolicLink(iNode, name);
}