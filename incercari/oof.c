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
   
 

lseek(fileDescriptor, 54, SEEK_SET);

	unsigned char pixel[3];

    while (read(fileDescriptor, pixel, 3) == 3)
{

	unsigned char grayValue = (unsigned char)(0.299 * pixel[2] + 0.587 * pixel[1] + 0.114 * pixel[0]);

	lseek(fileDescriptor, -3, SEEK_CUR);
	unsigned char grayPixel[3] = {grayValue, grayValue, grayValue};
	write(fileDescriptor, grayPixel, 3);
}
   
  
   close(fileDescriptor);

  printf("Conversia la gri pentru fisierul %s a fost facuta \n", inputPath);
    }
}

void process_file(char *inputPath, char *outputDirectory) {
  
    int readHeader;
    int fileDescriptor;
    int newFileDescriptor;
    int nrLinesWritten = 0;

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
	    
	    
		stat(inputPath, &fileStat); //se stocheaza informtii despre fisierul bmp in filestat

	  	time_t modificationTime = header.dataOffset;
	  	 struct tm *modificationTm = localtime(&modificationTime);
	   	char timeStr[20];
	   	strftime(timeStr, sizeof(timeStr), "%d.%m.%Y", modificationTm);
	   	 
	  

	    	int userId = getuid();

	    	char userPermissions[10];
	   	get_permissions(fileStat.st_mode, userPermissions);
	   	
	   	pid_t statChildPid = fork();

		if (statChildPid == -1) 
		{
	   	 perror("Eroare la fork pentru statistici");
	    	 exit(EXIT_FAILURE);
		} 
		else if (statChildPid == 0) 
		{

		if (S_ISREG(fileStat.st_mode)) 
		{
		      if (strstr(inputPath, ".bmp")) 
		    {
	  
			int pipe_fd[2];
			if (pipe(pipe_fd) == -1)
			{
				perror("Error creating pipe");
				exit(EXIT_FAILURE);
			}

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
		 nrLinesWritten++;
		 exit(nrLinesWritten);
	   }
	   
    int statusStatChild;
    waitpid(statChildPid, &statusStatChild, 0);

    if (WIFEXITED(statusStatChild)) 
    {
            int linesWrittenByChild = WEXITSTATUS(statusStatChild);

        printf("S-a încheiat procesul pentru statistici cu pid-ul %d și codul %d\n",statChildPid, WEXITSTATUS(statusStatChild));
                nrLinesWritten += linesWrittenByChild;  

    }

    if (strstr(inputPath, ".bmp")) 
    {
        pid_t bmpChildPid = fork();

        if (bmpChildPid == -1) 
        {
            perror("Eroare la fork pentru conversie la gri");
            exit(EXIT_FAILURE);
        } 
        else if (bmpChildPid == 0)
        {
            // Child process for BMP to grayscale conversion
            int grayscaleChildStatus = 0;
            convertToGrayscale(inputPath);
            exit(grayscaleChildStatus);
        }

        int statusBmpChild;
        waitpid(bmpChildPid, &statusBmpChild, 0);

        if (WIFEXITED(statusBmpChild)) 
        {
            printf("S-a încheiat procesul pentru conversie la gri cu pid-ul %d și codul %d\n", bmpChildPid, WEXITSTATUS(statusBmpChild));
        }
    }
    
     dprintf(newFileDescriptor, "Număr total de linii scrise: %d\n", nrLinesWritten);


    close(newFileDescriptor);
    close(fileDescriptor);
    closedir(outputDir);
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
        perror("Nu se poate crea sau deschide directorul de iesire");
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
                //copil
                printf("\n");
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
