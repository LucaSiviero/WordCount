#include <stdio.h>
#include <string.h>

typedef struct String{
    char* stringValue;
} String;


typedef struct dataset{
    int size;
    String words[60000];
    int count[];
} Dataset;

char* checkPunctuations(char string[]);

void checkExistance(Dataset *d);

char* checkPunctuations(char string[]) {
    for(int i = 0; i < strlen(string); i++) {
        string[i] = tolower(string[i]);

        switch (string[i]){
            case ',':
                string[i] = 0;
                break;

            case '\'':
                string[i] = ' ';
                for(i; i< strlen(string); i++) {
                    tolower(string[i]);
                    string[i] = string[i+1];
                    checkPunctuations(string);
                }
                break;
                
            case '.':
                string[i] = 0;
                break;
            
            case '!':
                string[i] = 0;
                break;

            case '?':
                string[i] = 0;
                break;
            
            case ':':
                string[i] = 0;
                break;

            case ';':
                string[i] = 0;
            break;

            case '\0':
                string[i] = 0;
        }
    }
    return string;
}


void checkExistance(Dataset* d){
    for(int i = 0; i < d->size; i++) {
        for (int j = i+1; j < d->size; j++) {
            if(strcmp(d->words[i].stringValue, d->words[j].stringValue) == 0 && (d->count[j] != 0)){        //Se la stringa è uguale, e non l'ho già considerata
                d->count[i] += d->count[j]; //Sommo le occorrenze
                d->count[j] = 0;      //E piazzo il counter dell'occorrenza ripetuta a 0
            }
        }
    }
}
