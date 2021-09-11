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
