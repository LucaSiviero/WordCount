#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "countLibrary.h"
#include "mpi.h"


int main(int argc, char* argv[]) {
    
    int rank, numtasks, tag = 1;
    
    char* str;
    int numWords = 0;
    int strLength;

    double start, end;
    double start_seq2, end_seq1, end_seq2;
    MPI_Init(NULL, NULL);
    MPI_Status stat;
    MPI_Request req;
    MPI_Comm_size( MPI_COMM_WORLD , &numtasks);
    MPI_Comm_rank( MPI_COMM_WORLD , &rank);

    MPI_Barrier(MPI_COMM_WORLD);        //Assicuriamoci che tutti i processi siano pronti a misurare lo stesso tempo


    if(rank == 0) {

        int remainder = 0;
        

        DIR *directory;
        printf("I want to know the name of the directory of files you want to execute word count on\n");
        scanf("%s", argv[0]);
        directory = opendir(argv[0]);
        start = MPI_Wtime();

        struct dirent *dir;

        int count = 0;
        String files[50];

        if(directory == NULL) {
            printf("No directory here\n");
        } 

        else {
            printf("Directory opened\n");
            while((dir = readdir(directory)) != NULL) {
                if(dir->d_type == DT_DIR) {}
                if(dir->d_type == DT_REG) { 
                    files[count].stringValue = dir->d_name;
                    count++;
                }
            }
        }

        char *str = malloc(50 * sizeof(char));
        FILE* fp;

        for(int i = 0; i < count; i++) {
            
            char str[80];
            strcpy(str, argv[0]);
            strcat(str, "/");
            strcat(str, files[i].stringValue);
            fp = fopen(str, "r");

            if(fp == NULL) {
                perror("Non ho aperto\n");
            }
            else {
                while(fscanf(fp, "%1023s", str) == 1) {
                    numWords++;
                }
                fclose(fp);
            }
        }


        printf("Number of words : %d\n", numWords);
        printf("Number of tasks without master : %d\n", numtasks - 1);


        remainder = numWords % (numtasks - 1);
        int last = 0;
        int portion = numWords / (numtasks - 1);

        printf("REMAINDER : %d\n", remainder);

        Dataset *container = malloc((sizeof(container) * numWords) + (numWords * sizeof(struct String)) + (numWords * sizeof(int)) * sizeof(int));
        container->size = 0;

        int j = 0;

        for(int i = 0; i < count; i++) {
            char str[80];
            strcpy(str, argv[0]);
            strcat(str, "/");
            strcat(str, files[i].stringValue);

            fp = fopen(str, "r");

            if(fp == NULL) {
                perror("Non ho aperto\n");
            }

            else {
                char word[50];
                while(fscanf(fp, "%1023s", word) == 1) {
                    char *word_checked = malloc(50 * sizeof(char));
                    word_checked = checkPunctuations(word);
                    int stringlen = strlen(word_checked);
                    container->words[j].stringValue = malloc(stringlen+1);
                    for(int i = 0; i < stringlen; i++) {
                        container->words[j].stringValue[i] = word_checked[i];
                    }
                    container->words[j].stringValue[stringlen] = '\0';
                    j++;
                    container->size++;
                }
            }
        }

        closedir(directory);

        for(int proc = 1; proc < numtasks; proc++) {
            if(proc <= remainder) {
                int final_portion = portion + 1;

                //FOR con last + portion e per i primi mandare portion + 1!
                int j = 0;
                for(int i = last; i < last + portion + 1; i++) {
                    j++;
                    int strLen = strlen(container->words[i].stringValue);
                    MPI_Send(&strLen, 1, MPI_INT, proc, 2, MPI_COMM_WORLD);
                    MPI_Send(container->words[i].stringValue, strLen, MPI_CHAR, proc, tag, MPI_COMM_WORLD);

                }
                j = 0;
                last += (final_portion);
            }

            if(proc > remainder) {

                for(int i = last; i < last + portion; i++) {
                    int strLen = strlen(container->words[i].stringValue);
                    MPI_Send(&strLen, 1, MPI_INT, proc, 3, MPI_COMM_WORLD);
                    MPI_Send(container->words[i].stringValue, strLen+1, MPI_CHAR, proc, tag, MPI_COMM_WORLD);

                }
                last += portion;
            }
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    end_seq1 = MPI_Wtime();
    MPI_Bcast(&numWords, 1, MPI_INT, 0, MPI_COMM_WORLD);


    if(rank > 0) {
        int strLen = 0;
        int remainder = 0;
        int portion = 0;
        int last = 0;
        remainder = numWords % (numtasks - 1);
        portion = numWords / (numtasks - 1);


        if(rank <= remainder) {
            portion += 1;

            Dataset *dt = malloc((sizeof(*dt) * portion) + (portion * sizeof(struct String)) +(portion * sizeof(int)) + sizeof(int));    //Istanziamo il dataset!

            dt->size = portion;
            printf("I'M RANK %d AND MY DATASET HAS SIZE %d\n", rank, dt->size);

            for(int i = 0; i < dt->size; i++) {
                MPI_Recv(&strLen, 1, MPI_INT, 0, 2, MPI_COMM_WORLD, &stat);
                dt->words[i].stringValue = malloc(strLen + 1);
                MPI_Recv(dt->words[i].stringValue, strLen, MPI_CHAR, 0, tag, MPI_COMM_WORLD, &stat);
                dt->words[i].stringValue[strLen] = '\0';
                dt->count[i] = 1;
            }

            
            checkExistance(dt);
            int howMany = 0;
            int stringlen = 0;
            for(int i = 0; i < dt->size; i++) {
                
                if(dt->count[i] != 0) {                     //QUI FAI IL PACKING DEI DATI DA INVIARE AL RANK 0. NEL RANK 0 FAI UNPACK E POI FAI LA COUNT FINALE
                    howMany++;
                }
            }
            MPI_Send(&howMany, 1, MPI_INT, 0, 5, MPI_COMM_WORLD);
            for(int i = 0; i < dt->size; i++) {
                if(dt->count[i] != 0) {
                    stringlen = strlen(dt->words[i].stringValue);
                    MPI_Send(&stringlen, 1, MPI_INT, 0, 10, MPI_COMM_WORLD);
                    MPI_Send(dt->words[i].stringValue, stringlen, MPI_CHAR, 0, tag, MPI_COMM_WORLD);
                    MPI_Send(&dt->count[i], 1, MPI_INT, 0, tag, MPI_COMM_WORLD);

                }
            }       
        }

        
        if(rank > remainder) {
            Dataset *dt = malloc((sizeof(*dt) * portion) + (portion * sizeof(struct String)) + (portion * sizeof(int)) * sizeof(int));    //Istanziamo il dataset!

            dt->size = portion;
            printf("OOO\n");

            printf("I'M RANK %d AND MY DATASET HAS SIZE %d\n", rank, dt->size);
            int count = 0;
            for(int i = 0; i < portion; i++) {

                MPI_Recv(&strLen, 1, MPI_INT, 0, 3, MPI_COMM_WORLD, &stat);
                dt->words[i].stringValue = malloc(strLen+1);

                MPI_Recv(dt->words[i].stringValue, strLen+1, MPI_CHAR, 0, tag, MPI_COMM_WORLD, &stat);
                count++;

                dt->words[i].stringValue[strLen] = '\0';
                dt->count[i] = malloc(sizeof(int));
                dt->count[i] = 1;
            }


            checkExistance(dt);

            int howMany = 0;
            int stringlen = 0;
            for(int i = 0; i < dt->size; i++) {
                if(dt->count[i] != 0) {                     //QUI FAI IL PACKING DEI DATI DA INVIARE AL RANK 0. NEL RANK 0 FAI UNPACK E POI FAI LA COUNT FINALE
                    howMany++;
                }
            }
            MPI_Send(&howMany, 1, MPI_INT, 0, 5, MPI_COMM_WORLD);

            for(int i = 0; i < dt->size; i++) {
                if(dt->count[i] != 0) {
                    stringlen = strlen(dt->words[i].stringValue);
                    MPI_Send(&stringlen, 1, MPI_INT, 0, 10, MPI_COMM_WORLD);
                    MPI_Send(dt->words[i].stringValue, stringlen, MPI_CHAR, 0, tag, MPI_COMM_WORLD);
                    MPI_Send(&dt->count[i], 1, MPI_INT, 0, tag, MPI_COMM_WORLD);
                }
            }
        }
    }


    MPI_Barrier(MPI_COMM_WORLD);
    start_seq2 = MPI_Wtime();
    if (rank == 0) {
        int howMany[numtasks - 1];
        int last = 0;
        Dataset *container = malloc(sizeof(*container) + numWords + (numWords * sizeof(struct String)) + (numWords * sizeof(int)) + sizeof(int));    //Istanziamo il dataset!
        container->size = 0;

        /*Uno ad uno, metti nel dataset tante word quanto è SIZE. Aggiorni le posizioni con last += SIZE. */
        for(int i = 1; i < numtasks; i++) {
            MPI_Recv(&howMany[i], 1, MPI_INT, i, 5, MPI_COMM_WORLD, &stat);
            container->size += howMany[i];

            printf("Receiveing back %d words from process %d\n", howMany[i], i);

            for(int j = last; j < last+howMany[i]; j++) {
                int stringlen = 0;
                MPI_Recv(&stringlen, 1, MPI_INT, i, 10, MPI_COMM_WORLD, &stat);
                container->words[j].stringValue = malloc(stringlen+1);

                MPI_Recv(container->words[j].stringValue, stringlen, MPI_CHAR, i, tag, MPI_COMM_WORLD, &stat);
                container->words[j].stringValue[stringlen] = '\0';
                container->count[j] = (int)malloc(sizeof(container->count[j]));
                MPI_Recv(&container->count[j], 1, MPI_INT, i, tag, MPI_COMM_WORLD, &stat);

            }

            last += howMany[i];
        }

        checkExistance(container);

        int size = 0;
        int total_words = 0;
        FILE* fp;
        fp = fopen("results.csv", "w+");
        fprintf(fp, "Count, Word\n");
        
        for(int k = 0; k < container->size; k++) {
            if(container->count[k] != 0){
                total_words += container->count[k];
                size++;
                printf("Count  :  %d  Word : %s\n", container->count[k], container->words[k].stringValue);
                fprintf(fp, "%d, %s\n", container->count[k], container->words[k].stringValue);
            }
        }
        fclose(fp);
        printf("Final size is : %d\n", size);
        printf("For a total of : %d words\n", total_words);
        end_seq2 = MPI_Wtime();
    }

    end = MPI_Wtime();

    if(rank == 0) {
        printf("Total execution time in ms : %f\n", end-start);
        printf("First partial execution time in ms : %f\nSecond partial execution time in ms : %f\n", end_seq1-start, end_seq2-start_seq2);

    }

    MPI_Finalize();
    return 0;
}


