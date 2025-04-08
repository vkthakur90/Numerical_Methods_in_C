#include <stdio.h>
#include <stdlib.h>
#include <omp.h>  // Include OpenMP header

#define MAX_DATA 100  // Define MAX_DATA as appropriate

typedef struct _ProgramData {
    size_t size;
    const char * filename; 
    struct {
        float principal[MAX_DATA];
        float rate[MAX_DATA];
        float time[MAX_DATA];
    } inputs;
    struct {
        float interest[MAX_DATA];
        float amount[MAX_DATA];  // Corrected spelling from 'ammount'
    } outputs;
} ProgramData;

void ProgramData_readInputsFromFile(struct _ProgramData * restrict data_ptr) {
    FILE *file = fopen(data_ptr->filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error: Could not open file '%s'.\n", data_ptr->filename);
        exit(EXIT_FAILURE);
    }

    // Read the size using the correct format specifier for size_t.
    if (fscanf(file, "%zu", &data_ptr->size) != 1) {
        fprintf(stderr, "Error: Could not read the size from file '%s'.\n", data_ptr->filename);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // Check that the size does not exceed the array capacity.
    if (data_ptr->size > MAX_DATA) {
        fprintf(stderr, "Error: Input size (%zu) exceeds maximum limit (%d) from file '%s'.\n", data_ptr->size, MAX_DATA, data_ptr->filename);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // Read each set of inputs from the file.
    for (size_t idx = 0; idx < data_ptr->size; ++idx) {
        if (fscanf(file, "%f", &data_ptr->inputs.principal[idx]) != 1) {
            fprintf(stderr, "Error: Could not read principal at index %zu.\n", idx);
            fclose(file);
            exit(EXIT_FAILURE);
        }
        if (fscanf(file, "%f", &data_ptr->inputs.rate[idx]) != 1) {
            fprintf(stderr, "Error: Could not read rate at index %zu.\n", idx);
            fclose(file);
            exit(EXIT_FAILURE);
        }
        if (fscanf(file, "%f", &data_ptr->inputs.time[idx]) != 1) {
            fprintf(stderr, "Error: Could not read time at index %zu.\n", idx);
            fclose(file);
            exit(EXIT_FAILURE);
        }
    }

    // Close the file after reading input.
    fclose(file);
}

void ProgramData_compute(struct _ProgramData * restrict data_ptr) {
    // Combine multi-threading and SIMD vectorization.
    // The 'parallel for simd' directive distributes loop iterations across threads
    // and instructs the compiler to vectorize the loop, if possible.
    #pragma omp parallel for simd schedule(static)
    for (size_t idx = 0; idx < data_ptr->size; ++idx) {
        // Compute the interest.
        float interest = 1.0f;
        interest *= data_ptr->inputs.principal[idx];
        interest *= data_ptr->inputs.rate[idx] / 100.0f;
        interest *= data_ptr->inputs.time[idx];
        data_ptr->outputs.interest[idx] = interest;
        
        // Compute the final amount by adding the computed interest.
        data_ptr->outputs.amount[idx] = data_ptr->inputs.principal[idx] + interest;
    }
}

void ProgramData_displayResults(struct _ProgramData * restrict data_ptr) {
    // Print the results sequentially.
    printf("-------------------------------------------------------------------------\n");
    printf("i\tPrincipal\tRate\tTime\t\tInterest\tAmount\n");
    printf("-------------------------------------------------------------------------\n");
    for (size_t i = 0; i < data_ptr->size; i++) {
        printf("|%2zu\t%.2f\t%.2f\t%.2f\t|\t%8.2f\t%8.2f|\n", 
                i, 
                data_ptr->inputs.principal[i], 
                data_ptr->inputs.rate[i], 
                data_ptr->inputs.time[i],
                data_ptr->outputs.interest[i], 
                data_ptr->outputs.amount[i]
        );
    }
    printf("-------------------------------------------------------------------------\n");
}

int main(int argc, char** argv) {
    // Validate command-line arguments.
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_filename>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    ProgramData data;
    data.filename = argv[1];
    
    ProgramData_readInputsFromFile(&data);
    ProgramData_compute(&data);
    ProgramData_displayResults(&data);
    
    return 0;
}
