#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mpi.h>

#define MAX_STRING_SIZE 100

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

    MPI_Init(NULL, NULL);
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
        printf("Encrypted string is : %s\n", encrypted_str);
        free(encrypted_str);
    }

    if (my_rank!=0){
        free(chunk);
    }
    MPI_Finalize();
}

int main(void){
    char *str = "This is a test stringg\n";
    encrypt_text(str);
}