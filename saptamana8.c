#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>
#include <limits.h>

typedef struct {
    int width;
    int height;
    int xPixelsPerM;
    int yPixelsPerM;
}InfoHeader;

typedef struct{
int signature;
int fileSize;
int reserved;
int dataOffset;
}Header;

typedef struct{
int red;
int green;
int blue;
int reserver;
}ColorTable;



void get_permissions(mode_t mode, char *str) {
    str[0] = (mode & S_IRUSR) ? 'R' : '-';
    str[1] = (mode & S_IWUSR) ? 'W' : '-';
    str[2] = (mode & S_IXUSR) ? 'X' : '-';
    str[3] = (mode & S_IRGRP) ? 'R' : '-';
    str[4] = (mode & S_IWGRP) ? 'W' : '-';
    str[5] = (mode & S_IXGRP) ? 'X' : '-';
    str[6] = (mode & S_IROTH) ? 'R' : '-';
    str[7] = (mode & S_IWOTH) ? 'W' : '-';
    str[8] = (mode & S_IXOTH) ? 'X' : '-';
    str[9] = '\0';
}

void convertToGrayscale(const char *inputPath) {
 int readHeader;
 

    int inputFile = open(inputPath, O_RDONLY);

    if (inputFile== -1) {
        perror("Error opening input file for conversion");
        exit(EXIT_FAILURE);
    }
    
    
 

 

   
int outputFile = open("fisiernou.bmp", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);


    if (outputFile== -1) {
        perror("Error creating output file for conversion");
        close(inputFile);
        exit(EXIT_FAILURE);
    }


    Header header;
    InfoHeader infoHeader;

   //citim header
  readHeader = read(inputFile, &header, sizeof(Header));

    if (readHeader== -1) {
        perror("Error reading header for conversion");
        close(inputFile);
        close(outputFile);
        exit(EXIT_FAILURE);
    }
    
    
  readHeader = read(inputFile, &infoHeader, sizeof(InfoHeader));

    if (readHeader== -1) {
        perror("Error reading header for conversion");
        close(inputFile);
        close(outputFile);
        exit(EXIT_FAILURE);
    }



    int imageSize = infoHeader.width * infoHeader.height;

    for (int i = 0; i < imageSize; ++i) {
        unsigned char pixel[3];
        read(inputFile, pixel, sizeof(pixel));

        //formula
        unsigned char grayValue = (unsigned char)(0.299 * pixel[0] + 0.587 * pixel[1] + 0.114 * pixel[2]);

        //scriem in fisierul de iesire
        write(outputFile, &grayValue, sizeof(grayValue));
    }

   
    close(inputFile);
    close(outputFile);

    printf("\n grayscale conversion for %s successful\n", inputPath);
}

void process_file(char *inputPath, char *outputDirectory) {
    char buffer[100000];
    int readHeader;
    int fileDescriptor;
    int newFileDescriptor;


    fileDescriptor= open(inputPath, O_RDWR);

    if (fileDescriptor == -1) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    Header header;
    InfoHeader infoHeader;

    readHeader = read(fileDescriptor, &header, sizeof(Header));

    if (readHeader == -1) {
        perror("Error reading header");
        close(fileDescriptor);
        exit(EXIT_FAILURE);
    }
    
    readHeader= read(fileDescriptor, &infoHeader, sizeof(InfoHeader));

    if (readHeader == -1) {
        perror("Eroare la citirea informatiilor despre imagine");
        close(fileDescriptor);
        exit(-1);
    }
    
    
    
    struct stat fileStat;

  

   
DIR *outputDir = opendir(outputDirectory);

if (outputDir == NULL) {
    perror("Error opening output directory");
    close(fileDescriptor);
    exit(EXIT_FAILURE);
} 

char statOutputPath[PATH_MAX];
char *inputFileName = strrchr(inputPath, '/'); //tai doar numele fisierului sa nu fie toata calea
if (inputFileName != NULL) {
    inputFileName++;  //sar peste /
} else {
    inputFileName = inputPath; //daca n-am / inseamna ca-s deja in directorul ala
}
snprintf(statOutputPath, PATH_MAX, "%s/%s_statistica.txt", outputDirectory, inputFileName);


newFileDescriptor = open(statOutputPath, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    if (newFileDescriptor == -1) 
    {
        perror("Error creating statistica.txt file");
        close(fileDescriptor);
        exit(EXIT_FAILURE);
    } 
    else 
    {
    
        stat(inputPath, &fileStat); //se stocheaza informtii despre fisierul bmp in filestat



        if (S_ISREG(fileStat.st_mode) && strstr(inputPath, ".bmp")) 
        {
            sprintf(buffer, "%d", infoHeader.width);
            write(newFileDescriptor, buffer, strlen(buffer));
            sprintf(buffer, "%d", infoHeader.height);
            write(newFileDescriptor, buffer, strlen(buffer));

            pid_t grayPid = fork();

            if (grayPid == -1) 
            {
                perror("Error creating child process for grayscale conversion");
                exit(EXIT_FAILURE);
            } 
            else if (grayPid == 0) 
            {
           
                close(newFileDescriptor);  
                convertToGrayscale(inputPath);
                exit(0);
            }
        }

        sprintf(buffer, "xPixelsPerM: %d \n", infoHeader.xPixelsPerM);
        write(newFileDescriptor, buffer, strlen(buffer));
        sprintf(buffer, "yPixelsPerM: %d \n", infoHeader.yPixelsPerM);
        write(newFileDescriptor, buffer, strlen(buffer));

        printf("Information saved in statistica.txt\n");
        
        int status;
        waitpid(-1, &status, 0);

        if (WIFEXITED(status)) {
            printf("Child process exited with code %d\n", WEXITSTATUS(status));
    }
}
    
   time_t modificationTime = header.dataOffset;
   struct tm *modificationTm = localtime(&modificationTime);
   char timeStr[20];
   strftime(timeStr, sizeof(timeStr), "%d.%m.%Y", modificationTm);
    
  

    int userId = getuid();

    char userPermissions[10];
   get_permissions(fileStat.st_mode, userPermissions);

    dprintf(newFileDescriptor, "File name: %s\n", inputPath);
    dprintf(newFileDescriptor, "Height: %d\n", infoHeader.height);
    dprintf(newFileDescriptor, "Width: %d\n", infoHeader.width);
    dprintf(newFileDescriptor, "File size: %ld\n", fileStat.st_size);
    dprintf(newFileDescriptor, "User ID: %d\n", userId);
    dprintf(newFileDescriptor, "Last modification time: %s\n", timeStr);
    dprintf(newFileDescriptor, "Link count: %ld\n", (long)fileStat.st_nlink);
    dprintf(newFileDescriptor, "User access rights: %s\n", userPermissions);
    dprintf(newFileDescriptor, "Group access rights: R--\n");
    dprintf(newFileDescriptor, "Others access rights: ---\n");

    close(newFileDescriptor);
    close(fileDescriptor);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        perror("Error, incorrect number of arguments");
        exit(EXIT_FAILURE);
    }

    char *inputDirectory = argv[1];
    char *outputDirectory = argv[2];

    DIR *dir = opendir(inputDirectory );

    if (dir == NULL) {
        perror("Error opening input directory");
        exit(EXIT_FAILURE);
    }

    // Create output directory if it does not exist
    if (mkdir(outputDirectory, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0 && errno != EEXIST) {
        perror("Error creating output directory");
        exit(EXIT_FAILURE);
    }

    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL) {
        char inputPath[PATH_MAX];
        snprintf(inputPath, PATH_MAX, "%s/%s", inputDirectory , entry->d_name);

        if (entry->d_type == DT_REG || entry->d_type == DT_LNK) 
        {

            pid_t pid = fork();

          if (pid == -1) 
            {
                perror("Error forking");
                exit(EXIT_FAILURE);
            } 
          else if (pid == 0) 
            {
                // Child
                process_file(inputPath, outputDirectory);
                exit(0);
            }

            int status;
            waitpid(pid, &status, 0);

            if (WIFEXITED(status)) {
                printf("Process with PID %d exited with code %d\n", pid, WEXITSTATUS(status));
            }
        }
    }

    closedir(dir);

    return 0;
}

