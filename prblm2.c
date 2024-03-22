#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mpi.h>

#define MAX_STRING_SIZE 100
#define MAX_FILE_SIZE 10000

void caeser_encrypt(char* str){
    int len = strlen(str);

    for (int i = 0; i < len; i++){
        if (str[i]>='A' && str[i]<='Z'){
            // capital letter
            str[i] = (str[i]-'A'+3)%26 + 'A';
        } else if (str[i]>='a' && str[i]<='z'){
            str[i] = (str[i]-'a'+3)%26 + 'a';
        } else {
            // non alphabetic
            str[i] = (char)str[i];
        }

    }
}


void caeser_decrypt(char* str){
    int len = strlen(str);

    for (int i = 0; i < len; i++){
        if (str[i]>='A' && str[i]<='Z'){
            // capital letter
            str[i] = (str[i]-'A'+ 26 - (3 % 26))%26 + 'A';
        } else if (str[i]>='a' && str[i]<='z'){
            str[i] = (str[i]-'a'+ 26 - (3 % 26))%26 + 'a';
        } else {
            // non alphabetic
            str[i] = (char)str[i];
        }

    }
}

void read_textfile(const char *filename, char *text){
    FILE *file;
    file = fopen(filename, "r");
    if (file==NULL){
        printf("Error opening the file!\n");
        exit(1);
    }

    int num_of_chars;
    fseek(file, 0, SEEK_END);
    num_of_chars = ftell(file);
    rewind(file);


    if (fread(text, 1, num_of_chars, file) != num_of_chars){
        printf("Error reading the file!\n");
        fclose(file);
        exit(1);
    }

    fclose(file);
    text[num_of_chars] = '\0';
}

int main(){

    char text[MAX_STRING_SIZE];
    int chunk_size, last_chunk_size;
    char encr_mode;

    MPI_Init(NULL, NULL);
    int comm_size, my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

    if (my_rank == 0){
        char read_mode;
        printf("Enter the desired mode => [f]ile or [c]onsole: ");
        scanf(" %c", &read_mode);

        printf("[e]ncrypt or [d]ecrypt? ");
        scanf(" %c", &encr_mode);
        if (encr_mode != 'e' && encr_mode != 'd') {
            printf("Invalid option\n");
            exit(1);
        }

        if (read_mode == 'f'){
            char filename[MAX_STRING_SIZE];
            printf("Enter the filename:\n");
            scanf(" %s", filename);
            read_textfile(filename, text);   
        } else if (read_mode == 'c'){
            printf("Enter the string:\n");
            scanf("%*[\n \t]");
            fgets(text, sizeof(text), stdin);
        } else {
            printf("Invalid Operation");
        }

        chunk_size = strlen(text)/(comm_size-1);
        last_chunk_size = chunk_size + strlen(text) % (comm_size-1);
        for (int i = 1; i < comm_size; i++){
            if (i == comm_size - 1)
                MPI_Send(&last_chunk_size, 1, MPI_INT, i, 3, MPI_COMM_WORLD);
            else
                MPI_Send(&chunk_size, 1, MPI_INT, i, 3, MPI_COMM_WORLD);
            MPI_Send(&encr_mode, 1, MPI_CHAR, i, 6, MPI_COMM_WORLD);
        }
    } else {
        MPI_Recv(&chunk_size, 1, MPI_INT, 0, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&encr_mode, 1, MPI_CHAR, 0, 6, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }


    if (my_rank == 0){
        for (int i = 1; i < comm_size; i++){
            if (i == comm_size - 1)
                MPI_Send(text + (i-1)*chunk_size, last_chunk_size, MPI_CHAR, i, 0, MPI_COMM_WORLD);
            else
                MPI_Send(text + (i-1)*chunk_size, chunk_size, MPI_CHAR, i, 0, MPI_COMM_WORLD);
        }
    } else {
        MPI_Recv(text, chunk_size, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        text[chunk_size] = '\0';
    }


    if (my_rank != 0){
        if (encr_mode == 'e'){
            caeser_encrypt(text);
        } else {
            caeser_decrypt(text);
        } 
        
        MPI_Send(text, chunk_size, MPI_CHAR, 0, 1, MPI_COMM_WORLD);
    } else { 
        for(int i = 1; i < comm_size; i++){
            if (i == comm_size - 1)
                MPI_Recv(&text[(i-1)*chunk_size], last_chunk_size, MPI_CHAR, i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            else
                MPI_Recv(&text[(i-1)*chunk_size], chunk_size, MPI_CHAR, i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        if (encr_mode == 'e'){
            printf("Encrypted string:\n%s\n", text);
        } else {
            printf("Decrypted string:\n%s\n", text);
        }
    }

    MPI_Finalize();
}
