
double time_difference(struct timespec* start, struct timespec* end);
struct readbuffer* setup_buffer(int buffersize);


struct readbuffer{
    char *buffer;
};