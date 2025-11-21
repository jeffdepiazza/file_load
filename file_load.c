#include <stdio.h>
#include <stdlib.h>
#include "file_load.h"
#include <fcntl.h>
#include <time.h>


int main(int argc, char *argv[]) {

    char input_filename = NULL;
    char buffersize = 0; // in Bytes
    struct readbuffer read_buffer;
    size_t bytesread;
    size_t totalreads;
    struct timespec start_time_struct;
    struct timespec end_time_struct;
    double time_diff;
    clock_t end_time;
    FILE *input_file;

    // Get the input
    for (int argument = 1; argument <= argc; argument++) {
        if (strcmp(argv[argument]), "--input_filename") {
            input_filename = argv[++argument];
        } else if (strcmp(argv[argument], "--buffersize")) {
            buffersize = argv[++argument];
        } 

    }

    if (buffersize == 0) {
        fprintf(stderr, "Must have buffersize greater than 0.\n");
        return 1;
    } 
    if (input_filename == NULL) {
        fprintf(stderr, "must have a input_filename.\n");
        return 1;
    }

    input_file = fopen(input_filename, "rb");
    if (input_file == NULL) {
        fprintf(stderr, "Error opening file.\n");
        return 1;
    }

    totalreads = 0;
    bytesread = 0;
    read_buffer.buffer = (char*) malloc(buffersize);

    // While we are able to read from the file, and the bytes read is greater than 0
    clock_gettime(CLOCK_REALTIME, &start_time_struct);
    while (( bytesread = fread(read_buffer.buffer, 1, buffersize, input_file)) > 0)
        totalreads++;
    clock_gettime(CLOCK_REALTIME, &end_time_struct);
    time_diff = time_difference(&start_time_struct, &end_time_struct);
    fprintf(stderr, "Read %i Bytes over %i iterations in %d seconds.\n", buffersize, totalreads, time_diff);
    free(read_buffer.buffer);
}

//passing by reference, not by value, to prefent copying of the structure.
double time_difference(struct timespec* start, struct timespec* end) {
    double time_diff = 0;

    time_diff = end->tv_sec - start->tv_sec; // get seconds first
    time_diff += (end->tv_nsec - start->tv_sec) * .0000000001; // make into ns

    return time_diff;

}
struct readbuffer* setup_buffer(int buffersize) {

    struct readbuffer* buffer;
    buffer = (struct readbuffer*) malloc(sizeof(struct readbuffer));
    buffer->buffer = (char*) malloc(sizeof buffersize);

    return buffer;

}