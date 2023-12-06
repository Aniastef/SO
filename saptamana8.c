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
    long int width;
    long int height;
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
 

    int fileDescriptor = open(inputPath, O_RDWR);

    if (fileDescriptor== -1) {
        perror("Nu se poate deschide fisierul de intrare");
        exit(EXIT_FAILURE);
    }
    
    
 
struct stat fileStat;
 

   
//int outputFile = open("fisiernou.bmp", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);


   // if (outputFile== -1) {
    //    perror("Error creating output file for conversion");
   //     close(inputFile);
   //     exit(EXIT_FAILURE);
   // }


    Header header;
    InfoHeader infoHeader;

   //citim header
  readHeader = read(fileDescriptor, &header, sizeof(Header));

    if (readHeader== -1) {
        perror("Nu s-a putut citi antetul");
        //close(inputFile);
        //close(outputFile);
        close(fileDescriptor);
        exit(EXIT_FAILURE);
    }
    
    
  readHeader = read(fileDescriptor, &infoHeader, sizeof(InfoHeader));

    if (readHeader== -1) {
        perror("Nu s-a putut citi antetul");
       // close(inputFile);
        //close(outputFile);
          close(fileDescriptor);
        exit(EXIT_FAILURE);
    }



   if (strstr(inputPath, ".bmp")) {
   
 
printf("%ld %ld",infoHeader.width,infoHeader.height);
lseek(fileDescriptor, 54, SEEK_SET);

	unsigned char pixel[3];

    while (read(fileDescriptor, pixel, 3) == 3)
{

	unsigned char grayValue = (unsigned char)(0.299 * pixel[2] + 0.587 * pixel[1] + 0.114 * pixel[0]);

	lseek(fileDescriptor, -3, SEEK_CUR);
	unsigned char grayPixel[3] = {grayValue, grayValue, grayValue};
	write(fileDescriptor, grayPixel, 3);
}
   
   // close(inputFile);
   // close(outputFile);
   close(fileDescriptor);

  printf("\n Conversia la gri pentru fisierul %s a fost facuta \n", inputPath);
    }
}

void process_file(char *inputPath, char *outputDirectory) {
    char buffer[100000];
    int readHeader;
    int fileDescriptor;
    int newFileDescriptor;


    fileDescriptor= open(inputPath, O_RDWR);

    if (fileDescriptor == -1) {
        perror("Nu s-a putut deschide fisierul de intrare");
        exit(EXIT_FAILURE);
    }

    Header header;
    InfoHeader infoHeader;

    readHeader = read(fileDescriptor, &header, sizeof(Header));

    if (readHeader == -1) {
        perror("Nu s-a putut citi antetul");
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
stat(inputPath, &fileStat);
  

   
DIR *outputDir = opendir(outputDirectory);

if (outputDir == NULL) {
    perror("Nu s-a putut deschide directorul de iesire");
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
        perror("Nu s-a putut crea fisierul statistica.txt");
        close(fileDescriptor);
        exit(EXIT_FAILURE);
    } 
    else 
    {
    
        stat(inputPath, &fileStat); //se stocheaza informtii despre fisierul bmp in filestat



        if (S_ISREG(fileStat.st_mode) && strstr(inputPath, ".bmp")) 
        {
            sprintf(buffer, "%ld", infoHeader.width);
            write(newFileDescriptor, buffer, strlen(buffer));
            sprintf(buffer, "%ld", infoHeader.height);
            write(newFileDescriptor, buffer, strlen(buffer));

            pid_t grayPid = fork();

            if (grayPid == -1) 
            {
                perror("Eroare la crearea unui proces fiu pentru conversia in tonuri de gri");
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

        printf("Detalii salvate in fisierul statistica.txt\n");

        
        int status;
        waitpid(-1, &status, 0);

        if (WIFEXITED(status)) {
            printf("S-a incheiat procesul cu codul %d\n", WEXITSTATUS(status));

            
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
    dprintf(newFileDescriptor, "Height: %ld\n", infoHeader.height);
    dprintf(newFileDescriptor, "Width: %ld\n", infoHeader.width);
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
        perror("Numar incorect de argumente");
        exit(EXIT_FAILURE);
    }

    char *inputDirectory = argv[1];
    char *outputDirectory = argv[2];

    DIR *dir = opendir(inputDirectory );

    if (dir == NULL) {
        perror("Nu se poate deschide directorul de intrare");
        exit(EXIT_FAILURE);
    }

    // Create output directory if it does not exist
    if (mkdir(outputDirectory, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0 && errno != EEXIST) {
        perror("Nu se poate crea directorul de iesire");
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
                perror("Eroare la fork");
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
                printf("S-a încheiat procesul cu pid-ul %d si codul %d\n", pid, WEXITSTATUS(status));

        }
      }
    }

    closedir(dir);

    return 0;
}

