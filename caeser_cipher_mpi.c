#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mpi.h>

#define MAX_STRING_SIZE 100
#define MAX_FILE_SIZE 10000

// takes a c-string and prints the encrypted string (caeser cipher: right shift of 3)
char* encrypt_string(const char* str){
    int len = strlen(str);
    char* encrypted_str = (char *)malloc((len+1)*sizeof(char));

    for (int i = 0; i < len; i++){
        if (str[i]>='A' && str[i]<='Z'){
            // capital letter
            encrypted_str[i] = (str[i]-'A'+3)%26 + 'A';
        } else if (str[i]>='a' && str[i]<='z'){
            encrypted_str[i] = (str[i]-'a'+3)%26 + 'a';
        } else {
            // non alphabetic
            encrypted_str[i] = (char)str[i];
        }

    }
    encrypted_str[len] = '\0';
    return encrypted_str;
}

void encrypt_text(char *inputStr){

    // input string divided into chunks of str
    int chunk_size;
    char *chunk;


    int comm_size, my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

    if (my_rank == 0){
        int len = strlen(inputStr);
        chunk_size = len/(comm_size-1) + 1;

        for (int i = 1; i < comm_size; i++){
            // tag 0 for len and tag 1 for portion
            MPI_Send(&chunk_size, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        }

        for (int i = 1; i < comm_size; i++){
            MPI_Send(&inputStr[(i-1)*chunk_size], chunk_size, MPI_CHAR, i, 1, MPI_COMM_WORLD);
        }
    } else {
        MPI_Recv(&chunk_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        chunk = (char*)malloc((chunk_size+1)*sizeof(char));
        MPI_Recv(chunk, chunk_size, MPI_CHAR, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        chunk[chunk_size] = '\0';
    }


    // encryption

    if (my_rank!=0){
        char *encrypted_chunk = encrypt_string(chunk);
        MPI_Send(encrypted_chunk, chunk_size, MPI_CHAR, 0, 3, MPI_COMM_WORLD);
        free(encrypted_chunk);
    } else {
        char *encrypted_str = (char*)malloc((chunk_size*comm_size+2)*sizeof(char));
        for (int i = 1; i < comm_size; i++){
            MPI_Recv(&encrypted_str[(i-1)*chunk_size], chunk_size, MPI_CHAR, i, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        encrypted_str[strlen(inputStr)] = '\0';
        printf("Encrypted string is : \n%s\n", encrypted_str);
        free(encrypted_str);
    }

    if (my_rank!=0){
        free(chunk);
    }
    
}


void encrypt_textfile(const char *filename){
    FILE *file;
    file = fopen(filename, "r");
    if (file==NULL){
        printf("Error opening the file!\n");
        exit(1);
    }

    // getting number of chars in the file
    char *buffer;
    int num_of_chars;
    fseek(file, 0, SEEK_END);
    num_of_chars = ftell(file);
    rewind(file);

    buffer = (char*)malloc(num_of_chars * sizeof(char));
    if (buffer == NULL){
        printf("Error allocating memory!\n");
        fclose(file);
        exit(1);
    }

    if (fread(buffer, 1, num_of_chars, file) != num_of_chars){
        printf("Error reading the file!\n");
        free(buffer);
        fclose(file);
        exit(1);
    }

    fclose(file);

    buffer[num_of_chars] = '\0';
    encrypt_text(buffer);
    free(buffer);
    
}


int main(void){
    MPI_Init(NULL, NULL);
    // int my_rank;
    // MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    encrypt_textfile("test.txt");

    // if (my_rank == 0){
    //     char read_mode;
    //     printf("Enter the desired mode => [f]ile or [c]onsole: ");
    //     fflush(stdout);
    //     scanf(" %c", &read_mode);

    //     char encr_mode;
    //     printf("[e]ncrypt or [d]ecrypt? ");
    //     scanf(" %c", &encr_mode);

    //     if (read_mode == 'f' && encr_mode == 'e'){
    //             char filename[MAX_STRING_SIZE];
    //             printf("Enter the filename:\n");
    //             scanf(" %s", filename);
    //             encrypt_textfile(filename);
    //     } else {
    //         printf("Invalid mode or operation!\n");
    //     }
    // }
    
    MPI_Finalize();
    return 0;
}