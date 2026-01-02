
#include <time.h>

double time_difference(struct timespec* start, struct timespec* end);
struct readbuffer* setup_buffer(int buffersize);
void run_via_read(int input_file, struct readbuffer* read_buffer);
void run_via_mmap(int input_file, struct readbuffer* read_buffer);
int create_test_file(char *filename, size_t filesize);

struct readbuffer{
    int buffersize;
    char *buffer;
};