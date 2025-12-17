#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <openssl/md5.h> 

#define NUM_WORKERS 8
#define MAX_PATH 1024
#define BUFFER_SIZE 4096 // to read files in 4KB chunks

typedef struct Node {
    char filepath[MAX_PATH];
    struct Node* next;
} Node;

Node* head = NULL;
Node* tail = NULL;

pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_cond = PTHREAD_COND_INITIALIZER;
int finished_scanning = 0;

// --- 2. Queue Functions (Same as before) ---
void enqueue(const char* path) {
    Node* new_node = malloc(sizeof(Node));
    if (!new_node) return; // Safety check
    strncpy(new_node->filepath, path, MAX_PATH);
    new_node->next = NULL;

    pthread_mutex_lock(&queue_mutex);
    if (tail == NULL) {
        head = new_node;
        tail = new_node;
    } else {
        tail->next = new_node;
        tail = new_node;
    }
    pthread_cond_signal(&queue_cond);
    pthread_mutex_unlock(&queue_mutex);
}

char* dequeue() {
    if (head == NULL) return NULL;
    
    Node* temp = head;
    char* path = strdup(head->filepath);
    head = head->next;
    if (head == NULL) tail = NULL;
    free(temp);
    return path;
}

// --- 3. Compute MD5 using OpenSSL (The NEW part) ---
void compute_md5(const char* filepath) {
    unsigned char c[MD5_DIGEST_LENGTH]; // Buffer for the final hash (16 bytes)
    char data[BUFFER_SIZE];             // Buffer to read file chunks
    FILE *inFile = fopen(filepath, "rb"); // Open in Binary mode
    MD5_CTX mdContext;                  // Structure to hold current MD5 state
    int bytes;

    if (inFile == NULL) {
        // printf("Could not open %s\n", filepath); // Optional error printing
        return;
    }

    // Initialize the library
    MD5_Init(&mdContext);

    // Read the file in chunks and feed it to the MD5 updater
    // This is memory efficient even for huge files (GBs in size)
    while ((bytes = fread(data, 1, BUFFER_SIZE, inFile)) != 0) {
        MD5_Update(&mdContext, data, bytes);
    }

    // Finish calculation
    MD5_Final(c, &mdContext);
    fclose(inFile);

    // Print filename first (as per prompt example)
    // The prompt output format: "filename <hash>"
    printf("%s ", filepath);

    // Print the hash (convert 16 raw bytes to 32 hex characters)
    for(int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        printf("%02x", c[i]);
    }
    printf("\n");
}

// --- 4. Worker Routine (Same as before) ---
void* worker_routine(void* arg) {
    while (1) {
        pthread_mutex_lock(&queue_mutex);

        while (head == NULL && !finished_scanning) {
            pthread_cond_wait(&queue_cond, &queue_mutex);
        }

        if (head == NULL && finished_scanning) {
            pthread_mutex_unlock(&queue_mutex);
            break;
        }

        char* file_to_process = dequeue();
        pthread_mutex_unlock(&queue_mutex);

        if (file_to_process != NULL) {
            compute_md5(file_to_process);
            free(file_to_process);
        }
    }
    return NULL;
}

// --- 5. Directory Traversal (Same as before) ---
void process_directory(const char* base_path) {
    struct dirent* entry;
    DIR* dp = opendir(base_path);

    if (dp == NULL) return;

    while ((entry = readdir(dp)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char path[MAX_PATH];
        snprintf(path, sizeof(path), "%s/%s", base_path, entry->d_name);

        struct stat statbuf;
        if (stat(path, &statbuf) == -1) continue;

        if (S_ISDIR(statbuf.st_mode)) {
            process_directory(path);
        } else {
            enqueue(path);
        }
    }
    closedir(dp);
}

// --- 6. Main (Same as before) ---
int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("USAGE: %s <directory/file> [more directories/files]\n", argv[0]);
        return 1;
    }

    pthread_t threads[NUM_WORKERS];

    for (int i = 0; i < NUM_WORKERS; i++) {
        pthread_create(&threads[i], NULL, worker_routine, NULL);
    }

    for (int i = 1; i < argc; i++) {
        struct stat statbuf;
        if (stat(argv[i], &statbuf) == 0) {
            if (S_ISDIR(statbuf.st_mode)) {
                process_directory(argv[i]);
            } else {
                enqueue(argv[i]);
            }
        }
    }

    pthread_mutex_lock(&queue_mutex);
    finished_scanning = 1;
    pthread_cond_broadcast(&queue_cond);
    pthread_mutex_unlock(&queue_mutex);

    for (int i = 0; i < NUM_WORKERS; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
