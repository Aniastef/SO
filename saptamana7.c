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


void process_file(char *inputPath) {
  
    int readHeader;
    int fileDescriptor;
    int newFileDescriptor;
    struct stat fileStat;

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
    
    newFileDescriptor = open("statistica.txt", O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    if (newFileDescriptor == -1) 
    {
        perror("Nu s-a putut crea fisierul statistica.txt");
        close(newFileDescriptor);
        exit(EXIT_FAILURE);
    } 
      else 
    {
    
        stat(inputPath, &fileStat); 

  	time_t modificationTime = header.dataOffset;
  	 struct tm *modificationTm = localtime(&modificationTime);
   	char timeStr[20];
   	strftime(timeStr, sizeof(timeStr), "%d.%m.%Y", modificationTm);
   	 
  

    	int userId = getuid();

    	char userPermissions[10];
   	get_permissions(fileStat.st_mode, userPermissions);

        if (S_ISREG(fileStat.st_mode)) 
        {
              if (strstr(inputPath, ".bmp")) 
            {
  
		dprintf(newFileDescriptor, "nume fisier: %s\n", inputPath);
		dprintf(newFileDescriptor, "inaltime: %ld\n", (long)fileStat.st_size);
		dprintf(newFileDescriptor, "lungime: %ld\n", (long)fileStat.st_size);
		dprintf(newFileDescriptor, "dimensiune: %ld\n", fileStat.st_size);
		dprintf(newFileDescriptor, "identificatorul utilizatorului: %d\n", userId);
		dprintf(newFileDescriptor, "timpul ultimei modificari: %s\n", timeStr);
		dprintf(newFileDescriptor, "contorul de legaturi: %ld\n",(long)fileStat.st_nlink);
		dprintf(newFileDescriptor, "drepturi de acces user: %s\n", userPermissions);
		dprintf(newFileDescriptor, "drepturi de acces grup: R--\n");
		dprintf(newFileDescriptor, "drepturi de acces altii: ---\n");

            } 
    	     else 
            {
                dprintf(newFileDescriptor, "nume fisier: %s\n", inputPath);
		dprintf(newFileDescriptor, "inaltime: %ld\n", (long)fileStat.st_size);
		dprintf(newFileDescriptor, "lungime: %ld\n", (long)fileStat.st_size);
		dprintf(newFileDescriptor, "dimensiune: %ld\n", fileStat.st_size);
		dprintf(newFileDescriptor, "identificatorul utilizatorului: %d\n", userId);
		dprintf(newFileDescriptor, "timpul ultimei modificari: %s\n", timeStr);
		dprintf(newFileDescriptor, "contorul de legaturi: %ld\n",(long)fileStat.st_nlink);
		dprintf(newFileDescriptor, "drepturi de acces user: %s\n", userPermissions);
		dprintf(newFileDescriptor, "drepturi de acces grup: R--\n");
		dprintf(newFileDescriptor, "drepturi de acces altii: ---\n");
      
   	     } 	
        }
    else if (S_ISDIR(fileStat.st_mode)) 
        {
		dprintf(newFileDescriptor, "nume director: %s\n", inputPath);
		dprintf(newFileDescriptor, "identificatorul utilizatorului: %d\n", userId);
		dprintf(newFileDescriptor, "drepturi de acces user: %s\n", userPermissions);
		dprintf(newFileDescriptor, "drepturi de acces grup: R--\n");
		dprintf(newFileDescriptor, "drepturi de acces altii: ---\n");
       	
         } 
    else if (S_ISLNK(fileStat.st_mode)) 
         {
        	char targetPath[PATH_MAX];
        	ssize_t targetSize = readlink(inputPath, targetPath, sizeof(targetPath) - 1);
        
	       	 if (targetSize == -1)
		{
		  perror("Nu se pot citi informatii despre legatura simboligca");
		  exit(EXIT_FAILURE);
		
		}
		targetPath[targetSize] = '\0';

		struct stat targetStat;
		
		if (lstat(targetPath, &targetStat) == -1) 
		{
		  perror("Nu s-au putut citi informatiile despre fisier");
		  exit(EXIT_FAILURE);
		}
		
		dprintf(newFileDescriptor, "nume legatura: %s\n", inputPath);
		dprintf(newFileDescriptor, "dimensiune: %ld\n", (long)fileStat.st_size);
		dprintf(newFileDescriptor, "dimensiune fisier: %ld\n", (long)targetStat.st_size);
    	
         }
   }

    close(newFileDescriptor);
    close(fileDescriptor);
}




  





int main(int argc, char *argv[]) {
    if (argc != 2) {
        perror("Eroare numar de argumente");
        exit(EXIT_FAILURE);
    }

    char *inputDirectory = argv[1];
    
   DIR *dir = opendir(inputDirectory);

    if (dir == NULL) {
        perror("Nu se poate deschide directorul de intrare");
        exit(EXIT_FAILURE);
    }


    int stats_fd = open("statistica.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (stats_fd == -1) {
        perror("Eroare la crearea fisierului de statistica");
        exit(EXIT_FAILURE);
    }


   struct dirent *entry;

    while ((entry = readdir(dir)) != NULL) {
        char inputPath[PATH_MAX];
        snprintf(inputPath, PATH_MAX, "%s/%s", inputDirectory , entry->d_name);

        if (entry->d_type == DT_REG || entry->d_type == DT_LNK) 
        {

           	process_file(inputPath);
           	exit(0);
         }
   
}

    closedir(dir);

    return 0;
}
