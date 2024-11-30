#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void passOne() {
    char label[10], opcode[10], operand[10], mnemonic[10];
    int start, locctr;
    char line[100];
    FILE *input, *optab, *symtab, *intermediate;

    input = fopen("pass1_input_code.txt", "r");
    optab = fopen("pass1_optab.txt", "r");
    symtab = fopen("pass1_symtab.txt", "w");
    intermediate = fopen("pass1_intermediate_code.txt", "w");

    if (!input || !optab || !symtab || !intermediate) {
        printf("Error: Could not open one or more files.\n");
        exit(1);
    }

    fgets(line, sizeof(line), input);
    sscanf(line, "%s %s %s", label, opcode, operand);
    if (strcmp(opcode, "START") == 0) {
        start = (int)strtol(operand, NULL, 16);
        locctr = start;
        fprintf(intermediate, "%04X\t%s\t%s\t%s\n", locctr, label, opcode, operand);
        printf("%04X\t%s\t%s\t%s\n", locctr, label, opcode, operand); // Display on terminal
    } else {
        locctr = 0;
    }

    while (fgets(line, sizeof(line), input) != NULL) {
        sscanf(line, "%s %s %s", label, opcode, operand);

        // Write to symtab only if there's a real label (not empty or "-")
        if (label[0] != '\0' && strcmp(label, "-") != 0) {
            fprintf(symtab, "%s\t%04X\n", label, locctr);
        }

        fprintf(intermediate, "%04X\t%s\t%s\t%s\n", locctr, label, opcode, operand);
        printf("%04X\t%s\t%s\t%s\n", locctr, label, opcode, operand); // Display on terminal

        // Check if opcode is format 4 (starts with '+')
        int format = 3;  // Default to format 3
        if (opcode[0] == '+') {
            format = 4; // Set format to 4 if opcode starts with '+'
            memmove(opcode, opcode + 1, strlen(opcode)); // Remove the '+' for lookup in OPTAB
        }

        // Search in OPTAB to check if the opcode exists
        rewind(optab);
        int found = 0;
        while (fgets(line, sizeof(line), optab) != NULL) {
            sscanf(line, "%s", mnemonic);
            if (strcmp(opcode, mnemonic) == 0) {
                found = 1;
                break;
            }
        }

        if (found) {
            // Check for format 2 opcodes
            if (strcmp(opcode, "CLEAR") == 0 || strcmp(opcode, "TIXR") == 0 || strcmp(opcode, "ADDR") == 0 ||
                strcmp(opcode, "SUBR") == 0 || strcmp(opcode, "MULR") == 0 || strcmp(opcode, "DIVR") == 0 ||
                strcmp(opcode, "COMPR") == 0 || strcmp(opcode, "SHIFTL") == 0 || strcmp(opcode, "SHIFTR") == 0 ||
                strcmp(opcode, "SVC") == 0 || (opcode[strlen(opcode) - 1] == 'R' && strlen(opcode) <= 5)) {
                format = 2;
            } else if (strcmp(operand, "-") == 0) {
                format = 1;
            }

            // Adjust locctr based on instruction format
            if (format == 1) {
                locctr += 1;
            } else if (format == 2) {
                locctr += 2;
            } else if (format == 4) {
                locctr += 4;
            } else {
                locctr += 3;
            }
        } else if (strcmp(opcode, "WORD") == 0) {
            locctr += 3;
        } else if (strcmp(opcode, "RESW") == 0) {
            locctr += 3 * atoi(operand);
        } else if (strcmp(opcode, "RESB") == 0) {
            locctr += atoi(operand);
        } else if (strcmp(opcode, "BYTE") == 0) {
            if (operand[0] == 'C') {
                locctr += strlen(operand) - 3;
            } else if (operand[0] == 'X') {
                locctr += (strlen(operand) - 3) / 2;
            }
        }

        if (strcmp(opcode, "END") == 0) {
            break;
        }
    }

    fclose(input);
    fclose(optab);
    fclose(symtab);
    fclose(intermediate);

    int length = locctr - start;
    printf("\nLength of the program is %04X\n", length);
    printf("Pass 1 completed successfully for SIC-XE.\n");
}

int main() {
    passOne();
    return 0;
}
