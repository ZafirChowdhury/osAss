#define OPENSSL_SUPPRESS_DEPRECATED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <openssl/md5.h>

#define N 10  // Buffer Size 
#define MAX_PATH 1024 


char buffer[N][MAX_PATH];
int in = 0;  // Points to next free slot
int out = 0; // Points to next filled slot


sem_t empty;            // Counts empty slots (Initial value = N)
sem_t full;             // Counts filled slots (Initial value = 0)
pthread_mutex_t mutex;  // Protects the buffer indices

int done = 0; // Flag to tell workers to stop

// MD5 Helper
void print_md5(char *filename) {
    unsigned char c[MD5_DIGEST_LENGTH];
    char data[1024];
    FILE *fp = fopen(filename, "rb");
    if (!fp) return;
    
    MD5_CTX ctx;
    MD5_Init(&ctx);
    int bytes;

    while ((bytes = fread(data, 1, 1024, fp)) != 0) {
        MD5_Update(&ctx, data, bytes);
    }

    MD5_Final(c, &ctx);
    fclose(fp);

    printf("%s ", filename);
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) printf("%02x", c[i]);
    printf("\n");
}

// CONSUMER
void *consumer(void *arg) {
    char my_file[MAX_PATH];
    
    while (1) {
        // If full <= 0 wait
        // If not go ahead
        sem_wait(&full);

        // Critical Section
        pthread_mutex_lock(&mutex);

        // Check if we are done and buffer is empty
        if (done && out == in) { 
            pthread_mutex_unlock(&mutex);
            sem_post(&full); // Wake up other sleepers so they can exit too
            break; 
        }

        // Per thread copy's file name into own memory then processes them

        // Copy fielname to local 
        strcpy(my_file, buffer[out]);
        out = (out + 1) % N;

        // EXIT Critical Section 
        pthread_mutex_unlock(&mutex);

        // Increments empty as it read 1
        sem_post(&empty);

        // Process MD5
        print_md5(my_file);
    }

    return NULL;
}

// The PRODUCER
void producer(char *path) {
    DIR *d = opendir(path);
    if (!d) return;
    
    struct dirent *dir;
    char fullpath[MAX_PATH];

    while ((dir = readdir(d)) != NULL) {
        // Skip hidden files and current dir
        if (dir->d_name[0] == '.') continue;  

        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, dir->d_name);
        
        // Copy file metadata to st
        struct stat st;
        stat(fullpath, &st);

        // If st is a directory
        if (S_ISDIR(st.st_mode)) {
            // Call itself with the directory
            producer(fullpath); 
        } 
        
        // If st is a file
        else {
            // Check if there is empty slot
            // If not wait
            // If yes decrement empty
            sem_wait(&empty);

            // Critical Section
            pthread_mutex_lock(&mutex);
            strcpy(buffer[in], fullpath);
            in = (in + 1) % N;
            pthread_mutex_unlock(&mutex);

            // Increments full by 1
            // To signal the consumer
            sem_post(&full);
        }
    }

    closedir(d);
}

int main(int argc, char *argv[]) {
    if (argc < 2) return 1;

    // Initialize Synchronization Tools
    sem_init(&empty, 0, N); // empty = 10
    sem_init(&full, 0, 0);  // full = 0
    pthread_mutex_init(&mutex, NULL);

    // Create 8 Consumers
    pthread_t threads[8];
    for (int i = 0; i < 8; i++) 
        pthread_create(&threads[i], NULL, consumer, NULL);

    // Run Producer (Main Thread)
    producer(argv[1]);

    // Cleanup
    pthread_mutex_lock(&mutex);
    done = 1;
    pthread_mutex_unlock(&mutex);
    
    // Wake up everyone so they can check the 'done' flag
    for (int i=0; i<8; i++) sem_post(&full); 

    for (int i = 0; i < 8; i++) 
        pthread_join(threads[i], NULL);

    return 0;
}
