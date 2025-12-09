#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>


int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: ./a <number>\n");
        return 1;
    }

    // converting arg to a int
    int n = atoi(argv[1]);

    // cheing if input is a positive int
    if (n <= 0) {
        printf("Incorrect input, input must be a positive integer\n");
        return 1;
    }

    pid_t pid = fork();

    // fork failed
    if (pid < 0) {
        perror("Fork failed"); // perror prints the specific system error
        return 1;
    }

    // child process
    else if (pid == 0) {
        printf("Sequence: %d", n);

        while (n != 1) {

            if (n % 2 == 0) {
                n = n / 2;
            } else {
                n = 3 * n + 1;
            }
            
            printf(", %d", n);
        }
        printf("\n");
    }

    // parent process
    else {
        // wait for the child process to complete
        wait(NULL);
    }

    return 0;
}
