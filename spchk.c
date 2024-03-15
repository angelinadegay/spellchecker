#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>


#include <dirent.h>

#define MAX_WORD_LENGTH 101// maximum word length + 1 for null terminator
#define TABLE_SIZE 100003 //number for the hash table size, a prime number

// node structure for linked list in each hash table entry
typedef struct Node
{
    char *word;
    struct Node *next;
} Node;

// hash table structure
Node *hashTable[TABLE_SIZE];

// creating node
Node *createNode(const char *word)
{
    Node *newNode = malloc(sizeof(Node));
    if (newNode == NULL)
    {
        printf("Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }
    newNode->word = strdup(word);
    newNode->next = NULL;
    return newNode;
}

// function prototype declarations
void processFile(const char *filePath, const char *dictionaryPath);     // process a single file
void processDirectory(const char *dirPath, const char *dictionaryPath); // process a directory
int isTextFile(const char *name);                                       // check if file is a text file
void checkSpelling(const char *filePath);                               // check spelling of a file
void loadDictionary(const char *dictionaryPath);                        // load dictionary into hash table
unsigned long hash(const char *word);                                   // hash function
bool findWord(const char *word);                                        // check if word exists in hash table
void insertWord(const char *word);                                      // insert word into hash table  
bool isWordInDictionary(const char *word);                              // check if word is in dictionary
char *stripPunctuation(char *word);                                     // strip punctuation from word
bool isHyphenated(const char *word);                                    // check if word is hyphenated
bool checkHyphenatedWord(const char *word);                             // check if hyphenated word is correct
void toAllCaps(const char *src, char *dest);                            // convert word to all caps
void toInitialCap(const char *src, char *dest);                         // convert word to initial cap

// hash function
unsigned long hash(const char *word)
{
    unsigned long hashValue = 5381;
    int c;

    while ((c = *word++))
        hashValue = ((hashValue << 5) + hashValue) + c; // hash * 33 + c

    return hashValue % TABLE_SIZE;
}

// checking if word exists in hash table
bool findWord(const char *word)
{
    unsigned long index = hash(word);
    Node *list = hashTable[index];
    while (list != NULL)
    {
        if (strcmp(list->word, word) == 0)
            return true; // word found
        list = list->next;
    }
    return false; // word not found
}
// insering word into hash table
void insertWord(const char *word)
{
    unsigned long index = hash(word);
    Node *newNode = createNode(word);
    // inserting at beginning of linked list 
    newNode->next = hashTable[index];
    hashTable[index] = newNode;
}

//loading dictionary and process words
void loadDictionary(const char *dictionaryPath)
{
    int fd = open(dictionaryPath, O_RDONLY);
    if (fd < 0)
    {
        write(STDERR_FILENO, "Failed to open dictionary file\n", 31);
        exit(EXIT_FAILURE);
    }

    char buffer[4096];
    ssize_t bytes_read;
    char word[MAX_WORD_LENGTH];  
    int wordIndex = 0; // index for the current word

    while ((bytes_read = read(fd, buffer, sizeof(buffer) - 1)) > 0)
    {
        for (int i = 0; i < bytes_read; ++i)
        {
            // checking for boundaries (space, newline, or EOF)
            if (buffer[i] == ' ' || buffer[i] == '\n' || buffer[i] == '\0')
            {
                if (wordIndex > 0)
                {                           
                    word[wordIndex] = '\0'; 

                    // inserting original word and its variations: initial cap and all caps
                    insertWord(word);
                    char initialCap[MAX_WORD_LENGTH];
                    char allCaps[MAX_WORD_LENGTH];
                    strcpy(initialCap, word);
                    strcpy(allCaps, word);
                    initialCap[0] = toupper(initialCap[0]);
                    toAllCaps(word, allCaps);
                    insertWord(initialCap);
                    insertWord(allCaps);

                    wordIndex = 0; // reset word index
                }
            }
            else
            {
                // appending character to word if it's not a boundary and we have space
                if (wordIndex < (sizeof(word) - 1))
                {
                    word[wordIndex++] = buffer[i];
                }
            }
        }
    }
    // checking for any remaining word in the buffer
    if (wordIndex > 0)
    {
        word[wordIndex] = '\0'; 
        insertWord(word);
        // inserting variations
        char initialCap[MAX_WORD_LENGTH];
        char allCaps[MAX_WORD_LENGTH];
        strcpy(initialCap, word);
        strcpy(allCaps, word);
        initialCap[0] = toupper(initialCap[0]);
        toAllCaps(word, allCaps);
        insertWord(initialCap);
        insertWord(allCaps);
    }

    if (bytes_read < 0)
    {
        write(STDERR_FILENO, "Failed to read dictionary file\n", 31);
        close(fd);
        exit(EXIT_FAILURE);
    }

    close(fd);
}

// processing directory
void processDirectory(const char *dirPath, const char *dictionaryPath)
{
    DIR *dir = opendir(dirPath);
    if (dir == NULL)
    {
        write(STDERR_FILENO, "Error: Unable to open directory\n", 32);
        return;
    }

    struct dirent *entry;
    char fullPath[1024];

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue; // skipping current and parent directory.
        }

        snprintf(fullPath, sizeof(fullPath), "%s/%s", dirPath, entry->d_name);

        if (entry->d_type == DT_DIR)
        {
            // if it's a directory, process it recursively.
            processDirectory(fullPath, dictionaryPath);
        }
        else if (isTextFile(entry->d_name))
        {
            // if it's a text file, check spelling.
            checkSpelling(fullPath);
        }
    }
    closedir(dir);
}

int isTextFile(const char *name)
{
    const char *dot = strrchr(name, '.');
    return dot && strcmp(dot, ".txt") == 0;
}

void processFile(const char *filePath, const char *dictionaryPath)
{
    if (!isTextFile(filePath))
    {
        write(STDERR_FILENO, "Error: Not a text file\n", 24);
        return;
    }
    checkSpelling(filePath);
}

void checkSpelling(const char *filePath)
{
    int fd = open(filePath, O_RDONLY);
    if (fd < 0)
    {
        write(STDERR_FILENO, "Failed to open file\n", 21);
        return;
    }

    char buffer[4096];
    ssize_t bytes_read;
    char wordBuffer[1024];
    int wordIndex = 0;
    unsigned long line = 1, column = 1, wordStartColumn = 1;

    while ((bytes_read = read(fd, buffer, sizeof(buffer) - 1)) > 0)
    {
        buffer[bytes_read] = '\0'; // null-terminate the buffer
        for (int i = 0; i < bytes_read; ++i)
        {
            // newline character found, increment line counter and reset column
            if (buffer[i] == '\n')
            {
                line++;
                column = 0;
            }
            else
            {
                column++;
            }

            if (isalnum(buffer[i]) || (wordIndex > 0 && (buffer[i] == '\'' || buffer[i] == '-')))
            {
                if (wordIndex == 0)
                    wordStartColumn = column; // marking the start of a word
                wordBuffer[wordIndex++] = buffer[i];
                if (wordIndex >= sizeof(wordBuffer) - 1)
                    break; // preventing buffer overflow
            }
            else if (wordIndex > 0)
            {
                wordBuffer[wordIndex] = '\0';
                char *cleanWord = stripPunctuation(wordBuffer); // cleaning the word

                if(isHyphenated(cleanWord))
                {
                    if (!checkHyphenatedWord(cleanWord))
                    {
                        printf("%s (%lu,%lu): %s\n", filePath, line, wordStartColumn, cleanWord);
                    }
                }
                else if (!isWordInDictionary(cleanWord))
                {
                    // reporting the error with the file, line, column, and word
                    printf("%s (%lu,%lu): %s\n", filePath, line, wordStartColumn, cleanWord);
                }

                wordIndex = 0; 
            }
        }
    }

    if (wordIndex > 0) // checking the last word in the buffer if there's no trailing space
    {
        wordBuffer[wordIndex] = '\0';
        char *cleanWord = stripPunctuation(wordBuffer); // cleaning the word
        if(isHyphenated(cleanWord))
        {
            if (!checkHyphenatedWord(cleanWord))
            {
                printf("%s (%lu,%lu): %s\n", filePath, line, wordStartColumn, cleanWord);
            }
        }
        else if (!isWordInDictionary(cleanWord))
        {
            // reporting the error with the file, line, column, and word
            printf("%s (%lu,%lu): %s\n", filePath, line, wordStartColumn, cleanWord);
        }
    }

    if (bytes_read < 0)
    {
        write(STDERR_FILENO, "Failed to read file\n", 21);
    }
    close(fd);
}

//helper functon: convert word to initial cap
void toInitialCap(const char *src, char *dest)
{
    bool isFirst = true;
    while (*src)
    {
        *dest++ = isFirst ? toupper((unsigned char)*src) : tolower((unsigned char)*src);
        isFirst = false;
        src++;
    }
    *dest = '\0'; // Null-terminate the string
}

// helper function: converting word to all caps
void toAllCaps(const char *src, char *dest)
{
    if (src == NULL || dest == NULL)
        return; 

    while (*src)
    {
        *dest++ = toupper((unsigned char)*src); // using toupper and then increment src
        src++;
    }
    *dest = '\0'; 
}

bool isWordInDictionary(const char *word)
{
    if (findWord(word))
    { 
        return true; //word found in dictionary
    }

    return false; // word not found in any variation
}
 // helper function: strip punctuation from word
char *stripPunctuation(char *word)
{
    size_t len = strlen(word);
    if (len == 0)
    {
        
        return word;
    }

    // finding the first alphanumeric character from the beginning
    size_t start = 0;
    while (start < len && !isalnum((unsigned char)word[start]))
    {
        start++;
    }

    // if the entire word is punctuation, returning an empty string
    if (start == len)
    {
        word[0] = '\0';
        return word;
    }

    // finding the first alphanumeric character from the end
    size_t end = len - 1;
    while (end > 0 && !isalnum((unsigned char)word[end]))
    {
        end--;
    }

    // shifting the alphanumeric content to the start of the string
    size_t i;
    for (i = 0; start <= end; start++, i++)
    {
        word[i] = word[start];
    }
    word[i] = '\0'; 
    return word;
}
// helper function: check if word is hyphenated
bool isHyphenated(const char *word)
{
    return strchr(word, '-') != NULL;
    

}
// helper function: check if hyphenated word is correct
bool checkHyphenatedWord(const char *word)
{
    // first, checking if the whole hyphenated word is in the dictionary.
    if (isWordInDictionary(word))
    {
        return true; // the entire hyphenated word is correct.
    }

    // if the whole word isn't found, proceeding to check individual components.
    // duplicating the word to avoid modifying the original string.
    char *temp = strdup(word);
    if (!temp)
    {
        //memory allocation failed
        return false;
    }

    bool allPartsCorrect = true;
    char *token = strtok(temp, "-");

    while (token != NULL)
    {
        if (!isWordInDictionary(token))
        {
            allPartsCorrect = false;
            break;
        }
        token = strtok(NULL, "-");
    }

    free(temp); // freeing the duplicated string
    return allPartsCorrect;
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        write(STDERR_FILENO, "Usage: ", 7);
        dprintf(STDERR_FILENO, "%s", argv[0]);
        write(STDERR_FILENO, " <dictionary_path> <text_file_or_directory_path>\n", 49);
        return 1;
    }

    struct stat pathStat;

    const char *dictionaryPath = argv[1];
    if (stat(dictionaryPath, &pathStat) != 0)
    {
        write(STDERR_FILENO, "Error: Unable to access dictionary file\n", 40);
        return 1;
    }
    else if (!S_ISREG(pathStat.st_mode))
    {
        write(STDERR_FILENO, "Error: The dictionary path provided is not a file\n", 51);
        return 1;
    }
    else
    {
        loadDictionary(dictionaryPath);
    }

    stat(argv[2], &pathStat);

    // checking if the path is a directory or a file
    if (S_ISDIR(pathStat.st_mode))
    {
        processDirectory(argv[2], dictionaryPath);
    }
    else if (S_ISREG(pathStat.st_mode))
    {
        processFile(argv[2], dictionaryPath);
    }
    else
    {
        write(STDERR_FILENO, "Error: The path provided is not a file or directory\n", 52);
        return 1;
    }

    return 0;
}