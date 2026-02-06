#include "arena.c"

#include <stdio.h>
#include <sys/mman.h>

#define ARENA_TEST_SIZE (MB(128))

static unsigned char backing_buf[ARENA_TEST_SIZE];

int main()
{
    struct Arena arena;

#ifdef LINUX_MMAP
    assert(arenaInit(&arena, ARENA_TEST_SIZE) == 0);
#else
    arenaInit(&arena, backing_buf, sizeof(backing_buf));
#endif

    void *initial_ptr = arenaPush(&arena, 0, 0);

    // Fill whole arena
    void *ptr = arenaPush(&arena, sizeof(backing_buf), 4);
    assert(ptr != NULL);
    assert((uintptr_t)ptr == (uintptr_t)initial_ptr);

    // Check no more space left
    ptr = arenaPush(&arena, 16, 2);
    assert(ptr == NULL);

    // Clear up whole arena
    arenaPop(&arena, arenaGetSize(&arena));
    ptr = arenaPush(&arena, 0, 0);
    assert(ptr != NULL);
    assert(initial_ptr == ptr);

    // Check that zeroing works the right way
    ptr = arenaPushZero(&arena, 16, 2);
    assert(ptr != NULL);
    assert((uintptr_t)ptr == (uintptr_t)initial_ptr);
    for (int i = 0; i < 16; i++)
    {
        assert(((unsigned char *)ptr)[i] == 0);
    }

    // Check alignment
    for (int i = 1; i < 1025; i = i * 2)
    {
        ptr = arenaPush(&arena, 128, i);
        assert(ptr != NULL);
        assert(((uintptr_t)ptr & (i - 1)) == 0);
    }
    
    // Try to read test file and compare it with what we got
    FILE *test_file = fopen("test_file.txt", "r");
    assert(test_file != NULL);
    void *test_file_mem = arenaPush(&arena, 128, 4);
    assert(test_file_mem != NULL);

    const char test_string[] = "Hello World";

    size_t read_sz = fread(test_file_mem, 1, strlen(test_string), test_file);
    assert(read_sz == strlen(test_string));
    ((unsigned char *)test_file_mem)[strlen(test_string)] = 0;
    assert(strncmp(test_file_mem, test_string, strlen(test_string) - 1) == 0);

#ifdef LINUX_MMAP
    assert(arenaRelease(&arena) == 0);
#else
    arenaInit(&arena, backing_buf, sizeof(backing_buf));
#endif

#ifdef LINUX_MMAP
    printf("All LINUX MMAP tests successfully passes!\n");
#else
    printf("All BACKING BUFFER tests successfully passes!\n");
#endif

    return 0;
}
