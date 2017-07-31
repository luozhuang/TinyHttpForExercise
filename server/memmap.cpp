#include "memmap.h"
#include <sys/mman.h>
#include <sys/stat.h>


Memorymap::~Memorymap()
{
    if (m_addr) {
        if (munmap(m_addr, m_len) == -1) {
            ERROR_HANDLE("detach memory error!");
        }
    }
    m_addr = nullptr;
    m_len = 0;
    if (shm_unlink(m_shmname) == -1) {
        ERROR_HANDLE("unlink memory error!");
    }
}

char* Memorymap::MapMemory(size_t len, int flags, mode_t mode)
{
    m_len = len;
    int shm_fd = shm_open(m_shmname, flags, mode);
    if (shm_fd < 0) {
        ERROR_HANDLE("open new shmfd error!");
    }

    m_addr = static_cast<char*>(mmap(nullptr, m_len, PROT_READ|PROT_WRITE,  MAP_SHARED, shm_fd, 0));
    if (m_addr == (void*)-1) {
        ERROR_HANDLE("map file failure!");
    }

    close(shm_fd);
    return m_addr;
}
char* Memorymap::MapFile(const char* filename, int flags, mode_t mode)
{
    int fd = open(filename, flags, mode);
    if (fd < 0) {
        ERROR_HANDLE("open new file error!");
    }

    struct stat file_stat;
    if (stat(filename, &file_stat) == -1) {
        ERROR_HANDLE("get file stat error!");
    }

    m_len = file_stat.st_size;


    m_addr = static_cast<char*>(mmap(nullptr, m_len, PROT_READ|PROT_WRITE,  MAP_SHARED, fd, 0));
    if (m_addr == (void*)-1) {
        ERROR_HANDLE("map file failure!");
    }
    close(fd);
    return m_addr;
}

void Memorymap::UnmapMemory()
{
    if (munmap(m_addr, m_len) == -1) {
        ERROR_HANDLE("detach memory error!");
    }

}
