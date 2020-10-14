#include "Disk.h"

const std::string Disk::GB = "GB";
const std::string Disk::MB = "MB";
const std::string Disk::KB = "KB";

Disk::Disk(std::string diskFileName) {
    LOG_INFO("Creating a new file system");
    this->diskFileName = normalizeName(diskFileName);

    if (access(this->diskFileName.c_str(), F_OK) == -1)
        format(DISK_SIZE);
    else loadFileSystemFromDisk();
    //printFileSystem();
}

std::string Disk::normalizeName(std::string name) {
    if (name.length() > (FILE_NAME_LEN - 1)) {
        name = name.substr(name.length() - FILE_NAME_LEN + 1,  name.length());
        name += '\0';
    }
    return name;
}

Disk::~Disk() {
    if (superBlock != NULL)
        delete superBlock;
    if (bitmap != NULL)
        delete[] bitmap;
    if (diskFile != NULL)
        fclose(diskFile);
}

void Disk::format(size_t diskSize) {
    LOG_INFO("Formatting disk");
    USER_ALERT("FORMATTING DISK (" + std::to_string(diskSize) + "B)");
    if (diskSize < (sizeof(SuperBlock_t) + sizeof(iNodes))) {
        USER_ALERT("CANNOT CREATE FILE");
        LOG_ERR("The size of the disk is too small");
        return;
    }
    initNewFileSystem(diskSize);
    USER_ALERT("OK");
}

void Disk::initNewFileSystem(size_t diskSize) {
    LOG_INFO("Creating a new file system");
    CLUSTER_COUNT = (diskSize - sizeof(SuperBlock_t) - sizeof(iNodes)) / (sizeof(bool) + CLUSTER_SIZE);

    diskFile = fopen(this->diskFileName.c_str(), "wb");
    ftruncate(fileno(diskFile), diskSize);
    rewind(diskFile);

    initNewSuperBlock(diskSize);
    initBitmap();
    initINodes();
    initializeRootINode();

    saveFileSystemOnDisk();
}

void Disk::initBitmap() {
    LOG_INFO("Creating a new bitmap");
    if (bitmap != NULL)
        delete[] bitmap;
    bitmap = new bool[CLUSTER_COUNT];
    for (int i = 0; i < CLUSTER_COUNT; i++)
        bitmap[i] = true;
}

void Disk::initNewSuperBlock(size_t diskSize) {
    LOG_INFO("Creating a new superblock");
    if (superBlock != NULL)
        delete superBlock;
    superBlock = new SuperBlock_t();

    strcpy(superBlock->signature,        SIGNATURE);
    strcpy(superBlock->volumeDescriptor, VOLUME_DESCRIPTION);

    superBlock->diskSize = diskSize;
    superBlock->clusterSize = CLUSTER_SIZE;
    superBlock->clusterCount = CLUSTER_COUNT;

    superBlock->bitmapStartAddr = sizeof(SuperBlock_t);
    superBlock->iNodeStartAddr = superBlock->bitmapStartAddr + (CLUSTER_COUNT * sizeof(bool));
    superBlock->dataStartAddr = superBlock->iNodeStartAddr + sizeof(iNodes);
}

void Disk::initINodes() {
    LOG_INFO("Initializing new i-nodes");
    for (int i = 0; i < INODES_COUNT; i++) {
        iNodes[i].nodeId = i;
        iNodes[i].parentId = NULL_POINTER;
        iNodes[i].size = 0;
        iNodes[i].isFree = true;
        iNodes[i].isDirectory = false;
        iNodes[i].isSymbolicLink = false;

        memset(iNodes[i].direct, NULL_POINTER, sizeof(iNodes[i].direct));
        memset(iNodes[i].indirect, NULL_POINTER, sizeof(iNodes[i].indirect));
    }
}

void Disk::saveFileSystemOnDisk() {
    LOG_INFO("Saving file system on the disk");
    diskFile = fopen(this->diskFileName.c_str(), "rb+");
    saveSuperblokOnDisk();
    saveRootDirectoryOnDisk();
    saveBitmapOnDisk();
    saveINodesOnDisk();
}

void Disk::saveSuperblokOnDisk() {
    LOG_INFO("Saving the superblock on the disk");
    fseek(diskFile, 0, SEEK_SET);
    fwrite(superBlock, sizeof(SuperBlock_t), 1, diskFile);
    fflush(diskFile);
}

void Disk::saveBitmapOnDisk() {
    LOG_INFO("Saving the bitmap on the disk");
    fseek(diskFile, superBlock->bitmapStartAddr, SEEK_SET);
    fwrite(bitmap, sizeof(bool), CLUSTER_COUNT, diskFile);
    fflush(diskFile);
}

void Disk::saveINodesOnDisk() {
    LOG_INFO("Saving the i-nodes on the disk");
    fseek(diskFile, superBlock->iNodeStartAddr, SEEK_SET);
    fwrite(&iNodes, sizeof(iNodes), 1, diskFile);
    fflush(diskFile);
}

void Disk::loadFileSystemFromDisk() {
    LOG_INFO("Loading file system from the disk");
    diskFile = fopen(this->diskFileName.c_str(), "rb+");
    loadSuperBlockFromDisk();
    loadBitmapFromDisk();
    loadINodesFromDisk();
    currentINode = &iNodes[ROOT_INODE_ID];
}

void Disk::loadSuperBlockFromDisk() {
    LOG_INFO("Loading a superblock from the disk");
    superBlock = new SuperBlock_t;
    fseek(diskFile, 0, SEEK_SET);
    fread(superBlock, sizeof(SuperBlock_t), 1, diskFile);
    CLUSTER_COUNT = (superBlock->diskSize - sizeof(SuperBlock_t) - sizeof(iNodes)) / (sizeof(bool) + CLUSTER_SIZE);
}

void Disk::loadBitmapFromDisk() {
    LOG_INFO("Loading a bitmap from the disk");
    bitmap = new bool[CLUSTER_COUNT];
    fseek(diskFile, superBlock->bitmapStartAddr, SEEK_SET);
    fread(bitmap, sizeof(bool), CLUSTER_COUNT, diskFile);
}

void Disk::loadINodesFromDisk() {
    LOG_INFO("Loading i-nodes from the disk");
    fseek(diskFile, superBlock->iNodeStartAddr, SEEK_SET);
    fread(iNodes, sizeof(iNodes), 1, diskFile);
}

void Disk::printFileSystem() {
    LOG_INFO("Printing out the file system");
    printSuperblock();
    printBitmap();
    printINodes();
}

std::string Disk::getCurrentPath() {
    //return currentPath;
    return getPath(currentINode);
}

void Disk::printCurrentDirectoryItems() {
    DirectoryItems_t *directoryItems = getDirectoryItemsFromINode(currentINode);
    printDirectoryItems(directoryItems);
    delete directoryItems;
}

void Disk::printDirectoryItems(const DirectoryItems_t *directoryItems) {
    std::cout << std::left << std::setw(10) << std::setfill(' ') << "size(B)";
    std::cout << std::left << std::setw(7) << std::setfill(' ') << "inode";
    std::cout << std::left << std::setw(8) << std::setfill(' ') << "p-inode\n";
    for (int i = 0; i < (int)directoryItems->count; i++)
        printDirectoryItem(&directoryItems->items[i]);
}

void Disk::printDirectoryItem(const DirectoryItem_t *directoryItem) {
    INode_t *iNode = &iNodes[directoryItem->iNode];
    std::cout << std::left << std::setw(10) << std::setfill(' ') << std::to_string(iNode->size);
    std::cout << std::left << std::setw(7) << std::setfill(' ') << iNode->nodeId;
    std::cout << std::left << std::setw(8) << std::setfill(' ') << iNode->parentId;
    if (iNode->isDirectory)
        std::cout << "[+] " << directoryItem->itemName;
    else {
        std::cout << "[-] " << directoryItem->itemName;
        if (iNode->isSymbolicLink == true) {
            std::cout << " -> ";
            printFileContent(iNode, false);
        }
    }
    std::cout << "\n";
}

void Disk::printSuperblock() const {
    std::cout << "<[SUPERBLOCK]>\n";
    std::cout << "signature:         " << superBlock->signature << "\n";
    std::cout << "volume descriptor: " << superBlock->volumeDescriptor << "\n";
    std::cout << "disk size:         " << superBlock->diskSize << "\n";
    std::cout << "cluster size:      " << superBlock->clusterSize << "\n";
    std::cout << "cluster count:     " << superBlock->clusterCount << "\n";
    std::cout << "bitmap address:    " << superBlock->bitmapStartAddr << "\n";
    std::cout << "i-nodes address:   " << superBlock->iNodeStartAddr << "\n";
    std::cout << "data address:      " << superBlock->dataStartAddr << "\n";
}

void Disk::printBitmap() const {
    std::cout << "<[BITMAP]>\n";
    for (int i = 0; i < CLUSTER_COUNT; i++)
        std::cout << (bitmap[i] ? "1" : "0");
    std::cout << "\n";
}

void Disk::printINodes() const {
    for (int i = 0; i < INODES_COUNT; i++) {
        printINode(&iNodes[i]);
        std::cout << "\n";
    }
}

void Disk::printINode(const INode_t *iNode) const {
    std::cout << "<[I-NODE]>\n";
    std::cout << "i-node id:        " << iNode->nodeId << "\n";
    std::cout << "i-node parent id: " << iNode->parentId << "\n";
    std::cout << "size:             " << iNode->size << "\n";
    std::cout << "free:             " << (iNode->isFree ? "true" : "false") << "\n";
    std::cout << "directory:        " << (iNode->isDirectory ? "true" : "false") << "\n";
    std::cout << "slink:            " << (iNode->isSymbolicLink ? "true" : "false") << "\n";
    for (int i = 0; i < NUM_OF_DIRECT_POINTERS; i++)
        std::cout << "direct (" << (i+1) << "):       " << iNode->direct[i] << "\n";
    for (int i = 0; i < NUM_OF_INDIRECT_POINTERS; i++)
        std::cout << "indirect (" << (i+1) << "):     " << iNode->indirect[i] << "\n";
}

void Disk::initializeRootINode() {
    LOG_INFO("Initializing a new root i-node");
    currentINode = &iNodes[ROOT_INODE_ID];

    currentINode->isFree = false;
    currentINode->isDirectory = true;
    currentINode->parentId = currentINode->nodeId;
}

int32_t Disk::getFreeCluster() {
    for (int32_t i = 0; i < CLUSTER_COUNT; i++)
        if (bitmap[i] == true) {
            bitmap[i] = false;
            return i;
        }
    return NULL_POINTER;
}

void Disk::saveRootDirectoryOnDisk() {
    LOG_INFO("Saving the root directory on the disk");
    auto rootDir = std::unique_ptr<DirectoryItems_t>(new DirectoryItems_t(currentINode->nodeId, currentINode->nodeId));
    currentINode->size = sizeof(size_t) + rootDir->count * sizeof(DirectoryItem_t);
    if (addDirectClustersToINode(iNodes) == false)
        return;
    saveDirectoryItemsOnDisk(currentINode, rootDir.get());
}

bool Disk::isThereAtLeastNFreeClusters(int32_t n) {
    LOG_INFO("Checking if there's at least n free clusters in the file system");
    int32_t freeClusters = 0;
    for (int i = 0; i < CLUSTER_COUNT; i++)
        if ((freeClusters += bitmap[i]) == n)
            return true;
    return false;
}

bool Disk::addDirectClustersToINode(INode_t *iNode) {
    LOG_INFO("Adding new direct clusters to the i-node");
    int32_t numberOfClusters = getNumberOfClustersNeeded(iNode->size);
    if (numberOfClusters > NUM_OF_DIRECT_POINTERS) {
        LOG_ERR("It requires to use indirect pointers as well");
        return false;
    }
    if (isThereAtLeastNFreeClusters(NUM_OF_DIRECT_POINTERS) == false) {
        LOG_ERR("There's not enough free clusters in the file system");
        return false;
    }
    for (int i = 0; i < NUM_OF_DIRECT_POINTERS; i++)
        iNode->direct[i] = getFreeCluster();
    return true;
}

Disk::DirectoryItems_t::DirectoryItems_t(int32_t iNodeId, int32_t iNodeParentId) {
    LOG_INFO("Creating an empty directory items");
    count = 2;
    items = new DirectoryItem_t[count];
    strcpy(items[0].itemName, ".");
    strcpy(items[1].itemName, "..");
    items[0].iNode = iNodeId;
    items[1].iNode = iNodeParentId;
}

Disk::DirectoryItems_t::~DirectoryItems_t() {
    if (items != NULL)
        delete[] items;
}

int32_t Disk::dataOffset(int32_t index) const {
    return superBlock->dataStartAddr + (index * superBlock->clusterSize);
}

int32_t Disk::getNumberOfClustersNeeded(int32_t size) const {
    if (size == 0)
        return 0;
    if (size < superBlock->clusterSize)
        return 1;
    int32_t numberOfClusters = size / superBlock->clusterSize;
    if (size % superBlock->clusterSize != 0)
        numberOfClusters++;
    return numberOfClusters;
}

void Disk::saveDirectoryItemsOnDisk(INode_t *iNode, DirectoryItems_t *directoryItems) {
    LOG_INFO("Saving directory items on the disk");
    if (iNode == NULL) {
        LOG_ERR("i-node is NULL");
        return;
    }
    if (directoryItems == NULL) {
        LOG_ERR("directory items is NULL");
        return;
    }

    int32_t numberOfClustersNeeded = getNumberOfClustersNeeded(iNode->size);
    int32_t numberOfItemsInCluster = (superBlock->clusterSize - sizeof(size_t)) / sizeof(DirectoryItem_t);

    if (numberOfClustersNeeded > NUM_OF_DIRECT_POINTERS) {
        LOG_ERR("It requires to use indirect pointers to store this directory as well");
        return;
    }
    LOG_INFO("Storing the number of directory items at the first position in the first cluster");
    fseek(diskFile, dataOffset(iNode->direct[0]), SEEK_SET);
    fwrite(&directoryItems->count, sizeof(size_t), 1, diskFile);
    fflush(diskFile);

    size_t index = 0;
    size_t count = 0;

    LOG_INFO("Storing the directory items themselves");
    DirectoryItem_t *buff = new DirectoryItem_t[numberOfItemsInCluster];
    for (int32_t i = 0; i < numberOfClustersNeeded; i++) {
        fseek(diskFile, dataOffset(iNode->direct[i]) + (i == 0 ? sizeof(size_t) : 0), SEEK_SET);
        for (int32_t j = 0; j < numberOfItemsInCluster; j++) {
            buff[j] = directoryItems->items[index++];
            count++;
            if (index == directoryItems->count)
                break;
        }
        fwrite(buff, sizeof(DirectoryItem_t), count, diskFile);
        count = 0;
    }
    fflush(diskFile);
    delete[] buff;
}

Disk::DirectoryItems_t * Disk::getDirectoryItemsFromINode(INode_t *iNode) {
    LOG_INFO("Loading directory items from the i-node");
    if (iNode == NULL) {
        LOG_ERR("the i-node is NULL");
        return NULL;
    }

    int32_t numberOfClustersNeeded = getNumberOfClustersNeeded(iNode->size);
    int32_t numberOfEntriesInCluster = (superBlock->clusterSize - sizeof(size_t)) / sizeof(DirectoryItem_t);

    if (numberOfClustersNeeded > NUM_OF_DIRECT_POINTERS) {
        LOG_ERR("To load this directory items requires to use indirect clusters as well");
        return NULL;
    }
    DirectoryItems_t *directoryItems = new DirectoryItems_t();

    LOG_INFO("Loading the number of items");
    fseek(diskFile, dataOffset(iNode->direct[0]), SEEK_SET);
    fread(&directoryItems->count, sizeof(size_t), 1, diskFile);

    directoryItems->items = new DirectoryItem_t[directoryItems->count];
    DirectoryItem_t *buff = new DirectoryItem_t[numberOfEntriesInCluster];

    LOG_INFO("Loading the items themselves");
    size_t index = 0;

    for (int32_t i = 0; i < numberOfClustersNeeded; i++) {
        fseek(diskFile, dataOffset(iNode->direct[i]) + (i == 0 ? sizeof(size_t) : 0), SEEK_SET);
        fread(buff, sizeof(DirectoryItem_t), numberOfEntriesInCluster, diskFile);

        for (int32_t j = 0; j < numberOfEntriesInCluster; j++) {
            directoryItems->items[index++] = buff[j];
            if (index == directoryItems->count)
                break;
        }
    }
    delete[] buff;
    return directoryItems;
}

Disk::INode_t *Disk::getFreeINode() {
    for (int i = 0; i < INODES_COUNT; i++)
        if (iNodes[i].isFree == true)
            return &iNodes[i];
    return NULL;
}

void Disk::incpyFile(INode_t *destinationINode, FILE *sourceFile, std::string fileName) {
    LOG_INFO("Copying the file into the file system");
    if (destinationINode == NULL) {
        USER_ALERT("PATH NOT FOUND");
        return;
    }
    if (destinationINode->isDirectory == false) {
        USER_ALERT("CANNOT IN-COPY INTO A FILE");
        return;
    }
    if (sourceFile == NULL) {
        USER_ALERT("FILE NOT FOUND");
        return;
    }
    INode_t *fileINode = getFreeINode();
    if (fileINode == NULL) {
        LOG_ERR("all i-nodes are occupied");
        return;
    }
    auto directoryItems = std::unique_ptr<DirectoryItems_t>(getDirectoryItemsFromINode(destinationINode));
    if (directoryItems == NULL) {
        LOG_ERR("Directory items is NULL");
        return;
    }
    if (existsInDirectory(directoryItems.get(), fileName) == true) {
        USER_ALERT("EXISTS");
        return;
    }

    LOG_INFO("Getting the size of the file");
    fseek(sourceFile, 0, SEEK_END);
    size_t fileSize = ftell(sourceFile);
    fseek(sourceFile, 0, SEEK_SET);

    int32_t numberOfClustersNeeded = getNumberOfClustersNeeded(fileSize);

    if (isThereAtLeastNFreeClusters(numberOfClustersNeeded) == false) {
        LOG_ERR("There's not enough free clusters in the file system");
        return;
    }

    LOG_INFO("Initialing free clusters for the file");
    std::vector<int32_t> clusters;
    for (int32_t i = 0; i < numberOfClustersNeeded; i++)
        clusters.push_back(getFreeCluster());

    auto buff = std::unique_ptr<char[]>(new char[superBlock->clusterSize + 1]);
    int32_t remainingFileSize = fileSize;

    LOG_INFO("Starting reading the content of the file");
    for (int32_t i = 0; i < numberOfClustersNeeded; i++) {
        fseek(sourceFile, i * superBlock->clusterSize, SEEK_SET);
        fread(buff.get(), sizeof(char), superBlock->clusterSize, sourceFile);

        fseek(diskFile, dataOffset(clusters[i]), SEEK_SET);
        if (remainingFileSize >= superBlock->clusterSize) {
            buff[superBlock->clusterSize] = '\0';
            fwrite(buff.get(), sizeof(char), superBlock->clusterSize, diskFile);
        }
        else {
            buff[remainingFileSize] = '\0';
            fwrite(buff.get(), sizeof(char), remainingFileSize, diskFile);
        }
        remainingFileSize -= superBlock->clusterSize;
    }
    fflush(diskFile);

    fileINode->size = fileSize;
    fileINode->isDirectory = false;
    fileINode->isFree = false;
    addINodeToDirectory(directoryItems.get(), destinationINode, fileINode, fileName);

    if (attachClustersToINode(fileINode, clusters) == false) {
        LOG_ERR("Attaching clusters to the i-node failed");
        return;
    }
    LOG_INFO("Storing the changes on the disk");
    saveBitmapOnDisk();
    saveINodesOnDisk();
    USER_ALERT("OK");
}

bool Disk::attachClustersToINode(INode_t *iNode, std::vector<int32_t> clusters) {
    LOG_INFO("Attaching clusters to the i-node");
    int32_t index = 0;
    int32_t numberOfPointersInCluster = superBlock->clusterSize / sizeof(int32_t);

    LOG_INFO("Attaching the direct pointers");
    for (; index < (int)clusters.size() && index < NUM_OF_DIRECT_POINTERS; index++)
        iNode->direct[index] = clusters[index];

    if (index < (int)clusters.size()) {
        LOG_INFO("Attaching the the first indirect pointer");
        if (isThereAtLeastNFreeClusters(1) == false) {
            LOG_ERR("There's not enough free clusters in the file system");
            return false;
        }

        int32_t count = 0;
        int32_t *clustersBuff = new int32_t[numberOfPointersInCluster];
        iNode->indirect[0] = getFreeCluster();

        for (; count < (int)numberOfPointersInCluster && index < (int)clusters.size(); count++)
            clustersBuff[count] = clusters[index++];

        LOG_INFO("Storing the clusters of the first indirect pointers");
        fseek(diskFile, dataOffset(iNode->indirect[0]), SEEK_SET);
        fwrite(clustersBuff, sizeof(int32_t), count, diskFile);
        fflush(diskFile);

        delete[] clustersBuff;
    }
    if (index < (int)clusters.size()) {
        LOG_INFO("Attaching the the second indirect pointer");

        int32_t numberOfRemainingClusters = clusters.size() - index + 1;
        int32_t numberOfMiddleClustersNeeded;

        if (numberOfRemainingClusters < numberOfPointersInCluster)
            numberOfMiddleClustersNeeded = 1;
        else {
            numberOfMiddleClustersNeeded = numberOfRemainingClusters / numberOfPointersInCluster;
            if (numberOfRemainingClusters % numberOfPointersInCluster != 0)
                numberOfMiddleClustersNeeded++;
        }
        if (isThereAtLeastNFreeClusters(numberOfMiddleClustersNeeded + 1) == false) {
            LOG_ERR("There's not enough free clusters in the file system");
            return false;
        }
        if (numberOfMiddleClustersNeeded > numberOfPointersInCluster) {
            LOG_ERR("The file is too big for this file system");
            return false;
        }
        LOG_INFO("Getting remaining free clusters");
        int32_t *middleClusters = new int32_t[numberOfMiddleClustersNeeded];
        for (int i = 0; i < numberOfMiddleClustersNeeded; i++)
            middleClusters[i] = getFreeCluster();

        LOG_INFO("Storing the middle clusters");
        iNode->indirect[1] = getFreeCluster();
        fseek(diskFile, dataOffset(iNode->indirect[1]), SEEK_SET);
        fwrite(middleClusters, sizeof(int32_t), numberOfMiddleClustersNeeded, diskFile);
        fflush(diskFile);

        int32_t count = 0;
        int32_t *clustersBuff = new int32_t[numberOfPointersInCluster];

        LOG_INFO("Started storing remaining clusters");
        for (int i = 0; i < numberOfMiddleClustersNeeded; i++) {
            for (int j = 0; j < numberOfPointersInCluster; j++) {
                if (index == (int)clusters.size())
                    break;
                clustersBuff[j] = clusters[index++];
                count++;
            }
            fseek(diskFile, dataOffset(middleClusters[i]), SEEK_SET);
            fwrite(clustersBuff, sizeof(int32_t), count, diskFile);
            count = 0;
        }
        fflush(diskFile);

        delete[] clustersBuff;
        delete[] middleClusters;
    }
    return true;
}

void Disk::addINodeToDirectory(DirectoryItems_t *directoryItems, INode_t *directoryINode, INode_t *newINode, std::string name) {
    LOG_INFO("Adding i-node into the directory");
    if (directoryItems == NULL) {
        LOG_ERR("Directory is NULL");
        return;
    }
    if (directoryINode == NULL) {
        LOG_ERR("Directory i-node is NULL");
        return;
    }
    if (newINode == NULL) {
        LOG_ERR("new i-node is NULL");
        return;
    }
    newINode->parentId = directoryINode->nodeId;
    directoryItems->count++;
    DirectoryItem_t *newEntries = new DirectoryItem_t[directoryItems->count];

    LOG_INFO("Adding a new item into the directory");
    newEntries[directoryItems->count - 1].iNode = newINode->nodeId;
    strcpy(newEntries[directoryItems->count - 1].itemName, name.c_str());

    LOG_INFO("Copying the rest of the items of the directory");
    for (size_t i = 0; i < directoryItems->count - 1; i++)
        newEntries[i] = directoryItems->items[i];

    LOG_INFO("Attaching the new items to the directory");
    DirectoryItem_t *tmp = directoryItems->items;
    directoryItems->items = newEntries;
    delete[] tmp;

    directoryINode->size = sizeof(size_t) + directoryItems->count * sizeof(DirectoryItem_t);
    saveDirectoryItemsOnDisk(directoryINode, directoryItems);
    saveINodesOnDisk();
}

bool Disk::existsInDirectory(DirectoryItems_t *directoryItems, std::string name) {
    LOG_INFO("Checking whether or not there's an item in the directory named " + name);
    if (directoryItems == NULL) {
        LOG_ERR("Directory is NULL");
        return false;
    }
    for (size_t i = 0; i < directoryItems->count; i++)
        if (std::string(directoryItems->items[i].itemName) == name)
            return true;
    return false;
}

std::vector<std::int32_t> Disk::getAllClustersOfINode(INode_t *iNode) {
    LOG_INFO("getting all clusters of the i-node");
    std::vector<std::int32_t> clusters;
    if (iNode == NULL) {
        LOG_ERR("The i-node is NULL");
        return clusters;
    }
    LOG_INFO("Getting direct clusters");
    for (int32_t i = 0; i < NUM_OF_DIRECT_POINTERS; i++) {
        if (iNode->direct[i] != NULL_POINTER)
            clusters.push_back(iNode->direct[i]);
        else return clusters;
    }

    int32_t numberOfClustersNeeded = getNumberOfClustersNeeded(iNode->size) - NUM_OF_DIRECT_POINTERS;
    int32_t numberOfPointersInCluster = superBlock->clusterSize / sizeof(int32_t);

    if (numberOfClustersNeeded <= 0)
        return clusters;

    int32_t count = std::min(numberOfClustersNeeded, numberOfPointersInCluster);
    int32_t *firstIndirectClusters = new int32_t[count];

    LOG_INFO("Getting first indirect clusters");
    fseek(diskFile, dataOffset(iNode->indirect[0]), SEEK_SET);
    fread(firstIndirectClusters, sizeof(int32_t), count, diskFile);

    for (int i = 0; i < count; i++)
        clusters.push_back(firstIndirectClusters[i]);
    delete[] firstIndirectClusters;

    if (numberOfClustersNeeded < numberOfPointersInCluster)
        return clusters;

    LOG_INFO("Getting second indirect clusters");
    numberOfClustersNeeded -= numberOfPointersInCluster;

    int32_t numberOfMiddleClusters;
    if (numberOfClustersNeeded < numberOfPointersInCluster)
        numberOfMiddleClusters = 1;
    else {
        numberOfMiddleClusters = numberOfClustersNeeded / numberOfPointersInCluster;
        if (numberOfClustersNeeded % numberOfPointersInCluster != 0)
            numberOfMiddleClusters++;
    }

    int32_t *middleClusters = new int32_t[numberOfMiddleClusters];
    int32_t *clustersBuff = new int32_t[numberOfPointersInCluster];

    fseek(diskFile, dataOffset(iNode->indirect[1]), SEEK_SET);
    fread(middleClusters, sizeof(int32_t), numberOfMiddleClusters, diskFile);

    for (int i = 0; i < numberOfMiddleClusters; i++) {
        count = std::min(numberOfClustersNeeded, numberOfPointersInCluster);
        fseek(diskFile, dataOffset(middleClusters[i]), SEEK_SET);
        fread(clustersBuff, sizeof(int32_t), count, diskFile);
        for (int j = 0; j < count; j++)
            clusters.push_back(clustersBuff[j]);
        numberOfClustersNeeded -= count;
    }
    delete[] middleClusters;
    delete[] clustersBuff;

    return clusters;
}

void Disk::outcpyFile(INode_t *sourceINode, FILE *destinationFile) {
    LOG_INFO("Out copying the file from the file system");
    if (sourceINode == NULL) {
        USER_ALERT("FILE NOT FOUND");
        return;
    }
    if (destinationFile == NULL) {
        USER_ALERT("PATH NOT FOUND");
        return;
    }
    if (sourceINode->isSymbolicLink) {
        std::string path = getPathFromSLink(sourceINode);
        INode_t *iNode = getINodeFromPath(path);
        outcpyFile(iNode, destinationFile);
    }
    else {
        char *buff = new char[superBlock->clusterSize];
        std::vector<int32_t> clusters = getAllClustersOfINode(sourceINode);
        int32_t remainingFileSize = sourceINode->size;

        for (int i = 0; i < (int) clusters.size(); i++) {
            fseek(diskFile, dataOffset(clusters[i]), SEEK_SET);
            fseek(destinationFile, i * superBlock->clusterSize, SEEK_SET);

            if (remainingFileSize >= superBlock->clusterSize) {
                fread(buff, sizeof(char), superBlock->clusterSize, diskFile);
                fwrite(buff, sizeof(char), superBlock->clusterSize, destinationFile);
            } else {
                fread(buff, sizeof(char), remainingFileSize, diskFile);
                fwrite(buff, sizeof(char), remainingFileSize, destinationFile);
            }
            remainingFileSize -= superBlock->clusterSize;
        }
        fflush(destinationFile);
        delete[] buff;
        USER_ALERT("OK");
    }
}

std::string Disk::getPathFromSLink(INode_t *iNode) {
    LOG_INFO("Getting the path of the i-node");
    if (iNode == NULL) {
        LOG_ERR("The i-node is NULL");
        return "";
    }
    if (iNode->isDirectory) {
        LOG_ERR("The i-node is NULL");
        return "";
    }
    if (iNode->isSymbolicLink == false) {
        LOG_ERR("The i-node is not a symbolic link");
        return "";
    }
    std::vector<int32_t> clusters = getAllClustersOfINode(iNode);
    char *buff = new char[superBlock->clusterSize + 1];
    int32_t remainingFileSize = iNode->size;
    std::stringstream ss;

    LOG_INFO("Starting printing out the content of the file");
    for (int32_t cluster : clusters) {
        fseek(diskFile, dataOffset(cluster), SEEK_SET);
        fread(buff, sizeof(char), superBlock->clusterSize, diskFile);

        if (remainingFileSize >= superBlock->clusterSize)
            buff[superBlock->clusterSize] = '\0';
        else buff[remainingFileSize] = '\0';
        remainingFileSize -= superBlock->clusterSize;
        ss << std::string(buff);
    }
    delete buff;
    return ss.str();
}

void Disk::printFileContent(INode_t *iNode, bool includeSlinks) {
    LOG_INFO("Printing out the content of the file");
    if (iNode == NULL) {
        USER_ALERT("FILE NOT FOUND");
        return;
    }
    if (iNode->isDirectory == true) {
        USER_ALERT("CANNOT PRINT OUT DIRECTORY");
        return;
    }
    if (includeSlinks && iNode->isSymbolicLink) {
        std::string path = getPathFromSLink(iNode);
        INode_t *fileINode = getINodeFromPath(path);
        printFileContent(fileINode, true);
    }
    else {
        std::vector<std::int32_t> clusters = getAllClustersOfINode(iNode);
        char *buff = new char[superBlock->clusterSize + 1];
        int32_t remainingFileSize = iNode->size;

        LOG_INFO("Starting printing out the content of the file");
        for (int32_t cluster : clusters) {
            fseek(diskFile, dataOffset(cluster), SEEK_SET);
            fread(buff, sizeof(char), superBlock->clusterSize, diskFile);

            if (remainingFileSize >= superBlock->clusterSize)
                buff[superBlock->clusterSize] = '\0';
            else buff[remainingFileSize] = '\0';
            remainingFileSize -= superBlock->clusterSize;
            std::cout << buff;
        }
        delete buff;
    }
}

void Disk::removeINodeFromParent(INode_t *iNode) {
    LOG_INFO("Removing the i-node from its parent");
    if (iNode == NULL) {
        LOG_ERR("The i-node is NULL");
        return;
    }
    INode_t *parentINode = &iNodes[iNode->parentId];
    DirectoryItems_t *parentDir = getDirectoryItemsFromINode(parentINode);
    if (parentDir == NULL) {
        LOG_ERR("The parent directory is NULL");
        return;
    }
    size_t position = 0;
    size_t index = 0;

    LOG_INFO("Finding an index of the file/folder within the directory");
    for (size_t i = 0; i < parentDir->count; i++)
        if (parentDir->items[i].iNode == iNode->nodeId) {
            position = i;
            break;
        }
    parentDir->count--;
    LOG_INFO("Copying all items into the new directory except the file/folder");
    DirectoryItem_t *newDirectoryItems = new DirectoryItem_t[parentDir->count];
    for (size_t i = 0; i < parentDir->count + 1; i++)
        if (i != position)
            newDirectoryItems[index++] = parentDir->items[i];

    DirectoryItem_t *tmp = parentDir->items;
    parentDir->items = newDirectoryItems;
    parentINode->size = sizeof(size_t) + parentDir->count * sizeof(DirectoryItem_t);

    LOG_INFO("Saving changes on the disk");
    delete[] tmp;
    saveDirectoryItemsOnDisk(parentINode, parentDir);
    saveINodesOnDisk();
    delete parentDir;
}

void Disk::removeINode(INode_t *iNode) {
    LOG_INFO("Removing the i-node");
    if (iNode == NULL) {
        LOG_ERR("The i-node is NULL");
        return;
    }
    LOG_INFO("Deleting all clusters of the i-node");
    std::vector<int32_t> clusters = getAllClustersOfINode(iNode);
    for (int32_t cluster : clusters)
        bitmap[cluster] = true;

    LOG_INFO("Deleting direct pointers");
    for (int i = 0; i < NUM_OF_DIRECT_POINTERS; i++) {
        if (iNode->direct[i] != NULL_POINTER) {
            bitmap[iNode->direct[i]] = true;
            iNode->direct[i] = NULL_POINTER;
        }
    }
    LOG_INFO("Deleting indirect pointers");
    for (int i = 0; i < NUM_OF_INDIRECT_POINTERS; i++) {
        if (iNode->indirect[i] != NULL_POINTER) {
            bitmap[iNode->indirect[i]] = true;
            iNode->indirect[i] = NULL_POINTER;
        }
    }
    LOG_INFO("Resetting the i-node");
    iNode->parentId = NULL_POINTER;
    iNode->size = 0;
    iNode->isFree = true;
    iNode->isDirectory = false;
    iNode->isSymbolicLink = false;

    saveINodesOnDisk();
    saveBitmapOnDisk();
}

void Disk::removeFile(INode_t *iNode) {
    LOG_INFO("Removing the file from the file system");
    if (iNode == NULL) {
        USER_ALERT("FILE NOT FOUND");
        return;
    }
    if (iNode->isDirectory) {
        USER_ALERT("TARGET IS NOT A FILE");
        return;
    }
    removeINodeFromParent(iNode);
    removeINode(iNode);
    USER_ALERT("OK");
}

void Disk::removeDirectory(INode_t *iNode) {
    LOG_INFO("Removing the directory from the file system");
    if (iNode == NULL) {
        USER_ALERT("FILE NOT FOUND");
        return;
    }
    if (iNode->isDirectory == false) {
        USER_ALERT("TARGET IS NOT A DIRECTORY");
        return;
    }
    if (iNode->nodeId == ROOT_INODE_ID) {
        USER_ALERT("CANNOT REMOVE ROOT DIRECTORY");
        return;
    }
    if (iNode->size != (sizeof(size_t) + 2 * sizeof(DirectoryItem_t))) {
        USER_ALERT("NOT EMPTY");
        return;
    }
    if (iNode->nodeId == currentINode->nodeId) {
        USER_ALERT("CANNOT REMOVE CURRENT DIRECTORY");
        return;
    }
    removeINodeFromParent(iNode);
    removeINode(iNode);
    USER_ALERT("OK");
}

Disk::INode_t *Disk::getINodeFromPath(std::string path) {
    LOG_INFO("Getting an i-node from the path");
    if (path.empty()) {
        LOG_ERR("The path is empty");
        return NULL;
    }
    if (path == "/")
        return &iNodes[ROOT_INODE_ID];
    if (path == "." || path == "./")
        return currentINode;
    if (path == ".." || path == "../")
        return &iNodes[currentINode->parentId];
    return getINodeFromPath(currentINode, path, path[0] != '/');
}

Disk::INode_t *Disk::getINodeFromPath(INode_t *iNode, std::string path, bool relative) {
    LOG_INFO("Getting an i-node from the path (relative/absolute)");
    DirectoryItems_t *dir = getDirectoryItemsFromINode(relative ? iNode : &iNodes[ROOT_INODE_ID]);
    if (dir == NULL) {
        LOG_ERR("The directory items is NULL");
        return NULL;
    }
    INode_t *targetINode = NULL;
    bool found;
    std::vector<std::string> parts = split(path, '/');

    LOG_INFO("Starting going through the path to find the target i-node");
    for (int i = 0; i < (int)parts.size(); i++) {
        found = false;
        for (size_t j = 0; j < dir->count; j++) {
            if (std::string(dir->items[j].itemName) == parts[i]) {
                targetINode = &iNodes[dir->items[j].iNode];
                if (i < (int)parts.size() - 1) {
                    if (targetINode->isDirectory == false) {
                        delete dir;
                        return NULL;
                    }
                    delete dir;
                    dir = getDirectoryItemsFromINode(targetINode);
                }
                found = true;
                break;
            }
        }
        if (found == false) {
            delete dir;
            return NULL;
        }
    }
    delete dir;
    return targetINode;
}

std::vector<std::string> Disk::split(const std::string& str, char separator) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (getline(ss, token, separator))
        if (token != "")
            tokens.emplace_back(token);
    return tokens;
}

void Disk::addNewFolder(std::string folderName) {
    addNewFolder(currentINode, folderName);
}

void Disk::addNewFolder(INode_t *destinationINode, std::string folderName) {
    LOG_INFO("Adding a new folder into the directory");
    if (destinationINode == NULL) {
        USER_ALERT("PATH NOT FOUND");
        return;
    }
    if (destinationINode->isDirectory == false) {
        USER_ALERT("TARGET IS NOT A DIRECTORY");
        return;
    }
    auto directory = std::unique_ptr<DirectoryItems_t>(getDirectoryItemsFromINode(destinationINode));
    if (directory == NULL) {
        LOG_ERR("Directory entries is NULL");
        return;
    }
    if (existsInDirectory(directory.get(), folderName)) {
        USER_ALERT("EXISTS");
        return;
    }
    INode_t *newFolderINode = getFreeINode();
    if (newFolderINode == NULL) {
        LOG_ERR("All i-nodes are occupied");
        return;
    }
    newFolderINode->isDirectory = true;
    newFolderINode->isFree = false;
    addDirectClustersToINode(newFolderINode);
    addINodeToDirectory(directory.get(), destinationINode, newFolderINode, folderName);

    DirectoryItems_t *newDir = new DirectoryItems_t(newFolderINode->nodeId, destinationINode->nodeId);
    newFolderINode->size = sizeof(size_t) + newDir->count * sizeof(DirectoryItem_t);

    saveDirectoryItemsOnDisk(newFolderINode, newDir);
    saveINodesOnDisk();
    saveBitmapOnDisk();
    delete newDir;
    USER_ALERT("OK");
}

void Disk::moveFileToADifferentDir(INode_t *fileINode, INode_t *destinationINode, std::string fileName) {
    LOG_INFO("Moving file to a different directory");
    if (fileINode == NULL) {
        USER_ALERT("FILE NOT FOUND");
        return;
    }
    if (fileINode->isDirectory == true) {
        USER_ALERT("CANNOT MOVE A DIRECTORY");
        return;
    }
    if (destinationINode == NULL) {
        USER_ALERT("PATH NOT FOUND");
        return;
    }
    if (destinationINode->isDirectory == false) {
        USER_ALERT("TARGET IS NOT A DIRECTORY");
        return;
    }
    if (fileName == "") {
        LOG_ERR("Name of the file is NULL");
        return;
    }
    auto destinationDir = std::unique_ptr<DirectoryItems_t>(getDirectoryItemsFromINode(destinationINode));
    if (destinationDir == NULL) {
        LOG_ERR("Destination directory is NULL");
        return;
    }
    if (existsInDirectory(destinationDir.get(), fileName)) {
        USER_ALERT("EXISTS");
        return;
    }

    removeINodeFromParent(fileINode);
    destinationDir = std::unique_ptr<DirectoryItems_t>(getDirectoryItemsFromINode(destinationINode));
    addINodeToDirectory(destinationDir.get(), destinationINode, fileINode, fileName);
    saveINodesOnDisk();
    USER_ALERT("OK");
}

void Disk::cd(std::string path) {
    LOG_INFO("Changing the current directory");
    INode_t *iNode = getINodeFromPath(path);
    if (iNode == NULL) {
        USER_ALERT("PATH NOT FOUND");
        return;
    }
    if (iNode->isDirectory == false) {
        USER_ALERT("TARGET IS NOT A DIRECTORY");
        return;
    }
    currentINode = iNode;
    USER_ALERT("OK");
}

void Disk::copyFileToADifferentDirectory(INode_t *fileINode, INode_t *destinationINode, std::string fileName) {
    LOG_INFO("Starting copying the file to the different directory");
    if (fileINode == NULL) {
        USER_ALERT("FILE NOT FOUND");
        return;
    }
    if (destinationINode == NULL) {
        USER_ALERT("PATH NOT FOUND");
        return;
    }
    if (fileINode->isDirectory == true) {
        USER_ALERT("CANNOT COPY A DIRECTORY");
        return;
    }
    if (destinationINode->isDirectory == false) {
        USER_ALERT("TARGET IS NOT A DIRECTORY");
        return;
    }
    if (fileName == "") {
        LOG_ERR("The Name of the file is NULL");
        return;
    }
    auto destinationDir = std::unique_ptr<DirectoryItems_t>(getDirectoryItemsFromINode(destinationINode));
    if (destinationDir == NULL) {
        LOG_ERR("The destination directory is NULL");
        return;
    }
    if (existsInDirectory(destinationDir.get(), fileName)) {
        USER_ALERT("EXISTS");
        return;
    }
    INode_t *newFileINode = getFreeINode();
    if (newFileINode == NULL) {
        LOG_ERR("All i-nodes are occupied");
        return;
    }
    std::vector<int32_t> clustersToCopy = getAllClustersOfINode(fileINode);

    if (isThereAtLeastNFreeClusters(clustersToCopy.size()) == false) {
        LOG_ERR("There is not enough free clusters in the file system");
        return;
    }

    LOG_INFO("copying clusters");
    std::vector<int32_t> newClusters;
    char *buff = new char[superBlock->clusterSize + 1];

    for (int i = 0; i < (int)clustersToCopy.size(); i++) {
        newClusters.push_back(getFreeCluster());
        fseek(diskFile, dataOffset(clustersToCopy[i]), SEEK_SET);
        fread(buff, sizeof(char), superBlock->clusterSize, diskFile);
        fseek(diskFile, dataOffset(newClusters[i]), SEEK_SET);
        fwrite(buff, sizeof(char), superBlock->clusterSize, diskFile);
    }
    fflush(diskFile);

    newFileINode->isDirectory = false;
    newFileINode->isFree = false;
    newFileINode->size = fileINode->size;
    newFileINode->isSymbolicLink = fileINode->isSymbolicLink;
    addINodeToDirectory(destinationDir.get(), destinationINode, newFileINode, fileName);

    if (attachClustersToINode(newFileINode, newClusters) == false) {
        LOG_ERR("Attaching clusters to the i-node failed");
        return;
    }
    delete[] buff;
    saveBitmapOnDisk();
    saveINodesOnDisk();
    USER_ALERT("OK");
}

void Disk::printInfoAboutINode(INode_t *iNode) {
    LOG_INFO("Printing info about the i-node");
    if (iNode == NULL) {
        USER_ALERT("FILE NOT FOUND");
        return;
    }
    printINode(iNode);
    std::cout << "clusters:  [";
    if (iNode->isDirectory == false) {
        std::vector<int32_t> clusters = getAllClustersOfINode(iNode);
        for (int i = 0; i < (int)clusters.size(); i++) {
            std::cout << clusters[i];
            if (i < (int)clusters.size() - 1)
                std::cout << " ";
        }
    }
    std::cout << "]\n";
}

std::string Disk::getPath(INode_t *iNode) {
    LOG_INFO("Getting path of the i-node");
    if (iNode == NULL) {
        LOG_ERR("The i-node is NULL");
    }

    INode_t *parent;
    DirectoryItems_t* directoryItems;
    std::stack<std::string> st;

    while (iNode->parentId != iNode->nodeId) {
        parent = &iNodes[iNode->parentId];
        directoryItems = getDirectoryItemsFromINode(parent);

        for (size_t i = 0; i < directoryItems->count; i++)
            if (directoryItems->items[i].iNode == iNode->nodeId) {
                st.push(std::string(directoryItems->items[i].itemName));
                break;
            }
        delete directoryItems;
        iNode = parent;
    }
    std::stringstream path;
    path << "/";
    while (!st.empty()) {
        path << st.top() << "/";
        st.pop();
    }
    return path.str();
}

void Disk::createSymbolicLink(INode_t *fileINode, std::string slinkName) {
    LOG_INFO("Creating a new symbolic link");
    if (fileINode == NULL) {
        USER_ALERT("FILE NOT FOUND");
        return;
    }
    if (fileINode->isDirectory) {
        USER_ALERT("TARGET IS NOT A FILE");
        return;
    }
    slinkName = normalizeName(slinkName);
    auto directoryItems = std::unique_ptr<DirectoryItems_t>(getDirectoryItemsFromINode(currentINode));
    if (directoryItems == NULL) {
        LOG_ERR("The directory items is NULL");
        return;
    }
    if (existsInDirectory(directoryItems.get(), slinkName)) {
        USER_ALERT("EXISTS");
        return;
    }
    INode_t *linkINode = getFreeINode();
    if (linkINode == NULL) {
        LOG_ERR("All i-nodes are occupied");
        return;
    }

    LOG_INFO("Changing the parameters of the i-node");
    std::string content = getPath(fileINode);
    content.pop_back();
    linkINode->isDirectory = false;
    linkINode->isFree = false;
    linkINode->isSymbolicLink = true;
    linkINode->size = content.length();
    addINodeToDirectory(directoryItems.get(), currentINode, linkINode, slinkName);

    LOG_INFO("Preparing clusters");
    int32_t numberOfClustersNeeded = getNumberOfClustersNeeded(content.length());
    if (isThereAtLeastNFreeClusters(numberOfClustersNeeded) == false) {
        LOG_ERR("There's not enough free clusters in the file system");
        return;
    }
    auto buff = std::unique_ptr<char[]>(new char[content.length()]);
    for (size_t i = 0; i < content.length(); i++)
        buff[i] = content[i];

    std::vector<int32_t> clusters;
    int32_t remainingSize = content.length();

    LOG_INFO("Storing data on the disk");
    for (int i = 0; i < numberOfClustersNeeded; i++) {
        clusters.push_back(getFreeCluster());
        fseek(diskFile, dataOffset(clusters[i]), SEEK_SET);
        if (remainingSize >= superBlock->clusterSize)
            fwrite(buff.get(), sizeof(char), superBlock->clusterSize, diskFile);
        else fwrite(buff.get(), sizeof(char), remainingSize, diskFile);
        remainingSize -= superBlock->clusterSize;
    }
    if (attachClustersToINode(linkINode, clusters) == false) {
        LOG_ERR("Attaching clusters to the i-node failed");
        return;
    }
    fflush(diskFile);
    saveBitmapOnDisk();
    saveINodesOnDisk();
    USER_ALERT("OK");
}