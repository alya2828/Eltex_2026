#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

volatile sig_atomic_t sigint_count = 0;
volatile sig_atomic_t got_sigquit = 0;
volatile sig_atomic_t exit_flag = 0;

void handler(int sig) {
    if (sig == SIGINT) {
        sigint_count++;
        if (sigint_count >= 3) {
            exit_flag = 1;
        }
    }
    else if (sig == SIGQUIT) {
        got_sigquit = 1;
    }
}

int main() {
    FILE *file = fopen("output.txt", "w");
    if (!file) {
        perror("fopen");
        return 1;
    }

    signal(SIGINT, handler);
    signal(SIGQUIT, handler);

    int counter = 0;

    while (!exit_flag) {
        counter++;

        fprintf(file, "%d\n", counter);
        fflush(file);

        if (got_sigquit) {
            fprintf(file, "SIGQUIT received\n");
            fflush(file);
            got_sigquit = 0;
        }

        sleep(1);
    }

    fprintf(file, "Program finished after 3 SIGINT\n");
    fclose(file);

    return 0;
}