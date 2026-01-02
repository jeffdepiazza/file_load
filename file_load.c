#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include "file_load.h"
#include <fcntl.h>
#include <time.h>


int main(int argc, char *argv[]) {

    char *input_filename = NULL;
    struct readbuffer *read_buffer;
    int method = 0; // 0 = mmap, 1 = read
    int input_file;  // File descriptor for input file
    int temp_buffersize = 0; //in Bytes
    int create_file = 0; // 0 = no, 1 = yes. Will create file specified by input_filename and then delete when done.

    // Get the input
    for (int argument = 1; argument < argc; argument++) {
        fprintf(stderr, "Parsing Argument %d: %s\n", argument, argv[argument]);
        if (strcmp(argv[argument], "--input_filename") == 0) {
            input_filename = argv[++argument];
            fprintf(stderr, "Input Filename: %s\n", input_filename);
        } else if (strcmp(argv[argument], "--buffersize") == 0) {
            temp_buffersize = atoi(argv[++argument]);
        } else if (strcmp(argv[argument], "--method") == 0) {
            if (strcmp(argv[++argument], "mmap") == 0) {
                method = 0;
            } else if (strcmp(argv[argument], "read") == 0) {
                method = 1;
            } else {
                fprintf(stderr, "Unknown method %s\n", argv[argument]);
                return 1;
            }
        } else if (strcmp(argv[argument], "--create_file") == 0) {
            create_file = 1;
        } else {
            fprintf(stderr, "Bad Option %s\n", argv[argument]);
            return 1;
        }
    }

    if (temp_buffersize == 0) {
        fprintf(stderr, "Must have buffersize greater than 0.\n");
        return 1;
    } 
    if (input_filename == NULL) {
        fprintf(stderr, "must have a input_filename.\n");
        return 1;
    }
    if (create_file == 1) {
        fprintf(stderr, "Creating test file %s\n", input_filename);
        input_file = create_test_file(input_filename, 1024 * 1024 * 20); // Create a 20MB test file
        if (input_file == -1) {
            fprintf(stderr, "Error creating test file.\n");
            return 1;
        }
    } else {
        input_file = open(input_filename, O_RDONLY);
    }

    if (input_file == -1) {
        fprintf(stderr, "Error opening file.\n");
        close(input_file);
        return 1;
    }

    //Peter showd me how to do this in one line that would only need one free call...can't remember it now though.
    read_buffer = setup_buffer(temp_buffersize);
    
    if (method == 0) {
        fprintf(stderr, "Running via mmap method.\n");
        run_via_mmap(input_file, read_buffer);
    } else if (method == 1) {
        fprintf(stderr, "Running via read method.\n");
        run_via_read(input_file, read_buffer);
    }

    close(input_file);
    if (create_file == 1) {
        // clean up the file next
        fprintf(stderr, "Removing test file %s\n", input_filename);
        remove(input_filename);
    }

    free(read_buffer->buffer);
    free(read_buffer);

}

//passing by reference, not by value, to prefent copying of the structure.
double time_difference(struct timespec* start, struct timespec* end) {
    double time_diff = 0;


    time_diff = end->tv_sec - start->tv_sec; // get seconds first
    time_diff += (end->tv_nsec - start->tv_nsec) * .0000000001; // make into ns

    return time_diff;

}
struct readbuffer* setup_buffer(int buffersize) {

    struct readbuffer* buffer;
    buffer = (struct readbuffer*) malloc(sizeof(struct readbuffer));
    if (buffer == NULL) {
        fprintf(stderr, "Error allocating memory for readbuffer struct.\n");
        return NULL;
    }
    buffer->buffer = (char*) malloc(sizeof(char) * buffersize);
    if (buffer->buffer == NULL) {
        fprintf(stderr, "Error allocating memory for readbuffer buffer.\n");
        free(buffer);
        return NULL;
    }
    buffer->buffersize = buffersize;

    return buffer;

}

void run_via_mmap(int input_file, struct readbuffer* read_buffer) {
    char* mapped_region;
    struct timespec start_time_struct;
    struct timespec end_time_struct;
    size_t bytesread = 0;
    size_t totalreads = 0;
    double time_diff = 0;

    fprintf(stderr, "Running with mmap.\n");
    mapped_region = mmap(NULL, read_buffer->buffersize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    
    if (mapped_region == MAP_FAILED) {
        fprintf(stderr, "Error mapping memory.\n");
        return;
    }
    // While we are able to read from the file, and the bytes read is greater than 0
    clock_gettime(CLOCK_REALTIME, &start_time_struct);
    while ((bytesread = read(input_file, mapped_region, (size_t) read_buffer->buffersize)) > 0)
        totalreads++;
    clock_gettime(CLOCK_REALTIME, &end_time_struct);
    time_diff = time_difference((struct timespec *) &start_time_struct, (struct timespec *) &end_time_struct);
    fprintf(stderr, "Read %i Bytes over %zu iterations in %f seconds.\n", read_buffer->buffersize, totalreads, time_diff);
   
    /* Unmap the region */
    if (munmap(mapped_region, (size_t) read_buffer->buffersize) == -1) {
        fprintf(stderr, "Error unmapping memory.\n");
        return;
    }   
}

void run_via_read(int input_file, struct readbuffer* read_buffer) {
    struct timespec start_time_struct;
    struct timespec end_time_struct;
    size_t bytesread = 0;
    size_t totalreads = 0;
    double time_diff = 0;

    fprintf(stderr, "Running with read.\n");
     // While we are able to read from the file, and the bytes read is greater than 0
    clock_gettime(CLOCK_REALTIME, &start_time_struct);
    while ((bytesread = read(input_file, read_buffer->buffer, read_buffer->buffersize)) > 0)
        totalreads++;
    clock_gettime(CLOCK_REALTIME, &end_time_struct);
    time_diff = time_difference((struct timespec *) &start_time_struct, (struct timespec *) &end_time_struct);
    fprintf(stderr, "Read %i Bytes over %zu iterations in %f seconds.\n", read_buffer->buffersize, totalreads, time_diff);
}

int create_test_file(char *filename, size_t filesize) {
    int file;
    file = open(filename, O_RDWR | O_CREAT, 0644);
    if (file == -1) {
        fprintf(stderr, "Error creating test file.\n");
        close(file);
        return -1;
    }
    char *text_to_input = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut \nlabore et dolore magna aliqua. Ut enim ad minim veniam,\n quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. \nDuis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. \nExcepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.\n";
    size_t text_length = strlen(text_to_input);
    size_t total_size_in_bytes = text_length * sizeof(char);
    size_t loops_needed = filesize / total_size_in_bytes;

    fprintf(stderr, "Creating test file of size %zu bytes with %zu loops of %zu bytes each.\n", filesize, loops_needed, total_size_in_bytes);
    for (size_t i = 0; i < loops_needed; i++) {
        write(file, text_to_input, total_size_in_bytes);
    }

    /* Rewind the file decriptor to the start so we can run this back through. */
    // Otherwise, the FD will be a the end of the file and remain there. */
    /* A '-1' value for the 'off_t' indicates an error.*/
    if (lseek(file, 0, SEEK_SET) == (off_t)-1) {
        fprintf(stderr, "Error rewinding file descriptor.\n");
        close(file);
        return -1;
    }
    return file;
}