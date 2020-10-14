#ifndef SETUP_H
#define SETUP_H

#define SIGNATURE_LEN   9   ///< size of signature of the owner of the file system
#define VOLUME_DESC_LEN 251 ///< size of the description of the file system
#define FILE_NAME_LEN   12  ///< size of a file name (11 + '\0'= 12B)

#define NUM_OF_DIRECT_POINTERS   5 ///< number of direct pointer of an i-node
#define NUM_OF_INDIRECT_POINTERS 2 ///< number of indirect pointer of an i-node

#define DISK_SIZE    50000000 ///< default size of the disk (50MB)
#define CLUSTER_SIZE 1000     ///< size of a cluster (1KB)
#define INODES_COUNT 100      ///< total number of i-nodes in the file system

#define SIGNATURE "silhavyj"  ///< signature of the owner of the file system
#define VOLUME_DESCRIPTION "ZOS project - A Simple File System Emulator" ///< a short description of the file system

#endif