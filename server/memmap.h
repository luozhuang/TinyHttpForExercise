#ifndef __MEMMAP_H__
#define __MEMMAP_H__

#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include "global.h"

class Memorymap
{
public:
    Memorymap(const char* shm_name = "/my_shm")
        :m_addr(nullptr),
        m_len(0),
        m_shmname(shm_name)
    {

    }

    ~Memorymap();

    void MapFile(const char* filename, int flags, mode_t mode);

    void MapMemory(size_t len, int flags, mode_t mode);
    void UnmapMemory();

private:
    char* m_addr;
    size_t m_len;
    const char* m_shmname;
};
#endif

