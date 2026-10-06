#include <stdlib.h>
#include <stdio.h>

#include <string.h>

int g_multiplier = 2;

typedef struct {
    int id;
    char label[4];
    double score;
} result_t;

result_t process_data(int id, const char* name, double base) {
    result_t r;
    r.id = id;

    strncpy(r.label, name, sizeof(r.label) - 1);
    r.label[sizeof(r.label) - 1] = '\0';
    
    r.score = (base * (double)g_multiplier);
    
    return r;
}

int main(int argc, char** argv) {
    char name[4] = {0};

    if (argc < 2) {
        printf("Usage: %s <name>\n", argv[0]);
        return EXIT_FAILURE;
    }

    strncpy(name, argv[1], sizeof(name) - 1);
    name[sizeof(name) - 1] = '\0';

    result_t result = process_data(101, argv[1], 4.5);

    for (int i = 0; i < g_multiplier; i++) {
        printf("Result [%d]: %s (Score: %.2f)\n", 
                result.id, result.label, result.score);
    }

    return EXIT_SUCCESS;
}