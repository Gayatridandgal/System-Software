#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void passTwo() {
    FILE *intermediate, *optab, *symtab, *objectProgram, *objcodeFile;
    char label[10], opcode[10], operand[10], mnemonic[10], line[100];
    int locctr, start = 0, operandAddress = 0, baseAddress = 0;
    char textRecord[70] = "", objectCode[10], tempObjectCode[10];
    int textRecordStart = 0, textRecordLength = 0;

    // Open files
    intermediate = fopen("pass1_intermediate_code.txt", "r");
    optab = fopen("pass1_optab.txt", "r");
    symtab = fopen("pass1_symtab.txt", "r");
    objectProgram = fopen("objectProgram.txt", "w");
    objcodeFile = fopen("objectCode.txt", "w");

    if (!intermediate || !optab || !symtab || !objectProgram || !objcodeFile) {
        printf("Error: Unable to open one or more files.\n");
        exit(1);
    }

    // Read the first line of the intermediate file
    fgets(line, sizeof(line), intermediate);
    sscanf(line, "%X %s %s %s", &locctr, label, opcode, operand);

    // Check if the first opcode is "START"
    if (strcmp(opcode, "START") == 0) {
        start = strtol(operand, NULL, 16);
        fprintf(objectProgram, "H^%-6s^%06X^%06X\n", label, start, 0); // Header record
        textRecordStart = start;
        fgets(line, sizeof(line), intermediate);
    }

    printf("Loc   Label    Opcode   Operand  ObjectCode\n");
    printf("-------------------------------------------\n");

    // Process each line of the intermediate file
    while (strcmp(opcode, "END") != 0) {
        sscanf(line, "%X %s %s %s", &locctr, label, opcode, operand);
        rewind(optab);
        int found = 0;

        // Search OPTAB for opcode
        while (fgets(tempObjectCode, sizeof(tempObjectCode), optab)) {
            sscanf(tempObjectCode, "%s %s", mnemonic, tempObjectCode);
            if (strcmp(opcode[0] == '+' ? opcode + 1 : opcode, mnemonic) == 0) {
                found = 1;
                break;
            }
        }

        if (found) {
            int format = (opcode[0] == '+') ? 4 : 3; // Determine instruction format
            int n = 1, i = 1, x = 0, b = 0, p = 0, e = 0;

            // Addressing modes
            if (operand[0] == '#') { // Immediate addressing
                n = 0;
                if (isdigit(operand[1])) {
                    operandAddress = atoi(operand + 1); // Direct constant
                } else {
                    rewind(symtab);
                    while (fgets(tempObjectCode, sizeof(tempObjectCode), symtab)) {
                        char symbol[10];
                        sscanf(tempObjectCode, "%s %X", symbol, &operandAddress);
                        if (strcmp(symbol, operand + 1) == 0) break;
                    }
                }
            } else if (operand[0] == '@') { // Indirect addressing
                i = 0;
                rewind(symtab);
                while (fgets(tempObjectCode, sizeof(tempObjectCode), symtab)) {
                    char symbol[10];
                    sscanf(tempObjectCode, "%s %X", symbol, &operandAddress);
                    if (strcmp(symbol, operand + 1) == 0) break;
                }
            } else { // Simple or indexed addressing
                char baseOperand[10];
                if (strchr(operand, ',') != NULL) { // Indexed addressing
                    sscanf(operand, "%[^,]", baseOperand);
                    x = 1;
                } else {
                    strcpy(baseOperand, operand);
                }
                rewind(symtab);
                while (fgets(tempObjectCode, sizeof(tempObjectCode), symtab)) {
                    char symbol[10];
                    sscanf(tempObjectCode, "%s %X", symbol, &operandAddress);
                    if (strcmp(symbol, baseOperand) == 0) break;
                }
            }

            // PC-relative or Base-relative addressing
            if (format == 3) {
                int disp = operandAddress - (locctr + 3);
                if (disp >= -2048 && disp <= 2047) {
                    p = 1;
                    operandAddress = disp & 0xFFF; // Mask to 12 bits
                } else if (baseAddress != 0 && operandAddress - baseAddress >= 0 && operandAddress - baseAddress <= 4095) {
                    b = 1;
                    operandAddress = (operandAddress - baseAddress) & 0xFFF;
                }
            } else if (format == 4) {
                e = 1; // Set extended format bit
            }

            // Generate object code
            int opcodeValue = strtol(tempObjectCode, NULL, 16);
            if (format == 4) {
                sprintf(objectCode, "%02X%01X%05X", opcodeValue + (n << 1) + i, x * 8 + b * 4 + p * 2 + e, operandAddress);
            } else {
                sprintf(objectCode, "%02X%01X%03X", opcodeValue + (n << 1) + i, x * 8 + b * 4 + p * 2 + e, operandAddress);
            }

            // Print object code
            fprintf(objcodeFile, "%04X\t%-8s\t%-8s\t%-8s\t%s\n", locctr, label, opcode, operand, objectCode);
            printf("%04X  %-8s%-8s%-8s%s\n", locctr, label, opcode, operand, objectCode);

            // Append object code to text record
            if (strlen(textRecord) + strlen(objectCode) > 60) {
                fprintf(objectProgram, "T^%06X^%02X^%s\n", textRecordStart, textRecordLength, textRecord);
                strcpy(textRecord, "");
                textRecordStart = locctr;
                textRecordLength = 0;
            }
            strcat(textRecord, objectCode);
            textRecordLength += strlen(objectCode) / 2;
        }

        fgets(line, sizeof(line), intermediate);
    }

    // Write the last text record and End record
    if (strlen(textRecord) > 0) {
        fprintf(objectProgram, "T^%06X^%02X^%s\n", textRecordStart, textRecordLength, textRecord);
    }
    fprintf(objectProgram, "E^%06X\n", start);

    fclose(intermediate);
    fclose(optab);
    fclose(symtab);
    fclose(objectProgram);
    fclose(objcodeFile);

    printf("Pass 2 completed successfully for SIC/XE.\n");
}
int main()
{
     passTwo();
    return 0;
}
