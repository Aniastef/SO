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



struct BMPHeader {
    int file_size;
    int data_offset;
    int header_size;
    int width;
    int height;
};

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


//void convert_to_grayscale(const char *input_path, const char *output_path){}


int main(int argc, char *argv[]) {
    if (argc != 3) {
        perror("Eroare numar de argumente");
        exit(EXIT_FAILURE);
    }

    char *input_directory = argv[1];
    char *output_directory=argv[2];

 
  
    //deschidere director iesire
    if (mkdir(output_directory, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0 && errno != EEXIST) {
        perror("Eroare la crearea directorului de ieșire");
        exit(EXIT_FAILURE);
    }

    //deschidere director intrare
    DIR *dir = opendir(input_directory);
    if (dir == NULL) {
        perror("Eroare deschidere director de intrare");
        exit(EXIT_FAILURE);
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
      if (entry->d_type == DT_REG)
	{
	pid_t pid = fork();
	if (pid == -1) {
	  perror("Eroare la fork");
	  exit(EXIT_FAILURE);	  
	}
	else if (pid == 0) {
	  //fiu
	  char input_path[PATH_MAX];
	  char output_path[PATH_MAX];

	  
	  snprintf(input_path, PATH_MAX, "%s/%s", input_directory, entry->d_name);
	  snprintf(output_path, PATH_MAX, "%s/%s_statistica.txt", output_directory, entry->d_name);

	  //deschidere statistica
	  int stats_fd = open(output_path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	  if (stats_fd == -1) {
	    perror("Error creating the statistics file");
	    exit(EXIT_FAILURE);
}

	   if (strstr(entry->d_name, ".bmp") != NULL) {
          
                    pid_t bmp_pid = fork();
                    if (bmp_pid == -1) {
                        perror("Eroare la fork pentru imaginea BMP");
                        exit(EXIT_FAILURE);
                    } else if (bmp_pid == 0) {
                        //fiu
		      // convert_to_grayscale(input_path, output_path);
                        exit(EXIT_SUCCESS);
                    }
                }

	   

//deschidre bmp
    int fd = open(input_path, O_RDONLY);
    if (fd == -1) {
        perror("Eroare la deschiderea fisierului");
    }

int fo = open(output_path, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fo == -1) {
        perror("Eroare la deschiderea fisierului");
    }
    
    struct BMPHeader bmp_header;
    
    //citire header
    if (read(fd, &bmp_header, sizeof(bmp_header)) != sizeof(bmp_header)) {
        perror("Eroare la citirea header-ului BMP");
        close(fd);
    }

    //informatii header
    int height = bmp_header.height;
    int width = bmp_header.width;
    int file_size = bmp_header.file_size;
    int user_id = getuid();

   
    close(fd);

    time_t modification_time = bmp_header.data_offset; 
    struct tm *modification_tm = localtime(&modification_time);
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%d.%m.%Y", modification_tm);


    char user_permissions[10];
    get_permissions(S_IRUSR | S_IWUSR | S_IXUSR, user_permissions);
    
    //ce scriem in statistica.txt
    dprintf(stats_fd, "nume fisier: %s\n", output_path);
    dprintf(stats_fd, "inaltime: %d\n", height);
    dprintf(stats_fd, "lungime: %d\n", width);
    dprintf(stats_fd, "dimensiune: %d\n", file_size);
    dprintf(stats_fd, "identificatorul utilizatorului: %d\n", user_id);
    dprintf(stats_fd, "timpul ultimei modificari: %s\n", time_str);
    dprintf(stats_fd, "contorul de legaturi: %ld\n", (long)file_size); 
    dprintf(stats_fd, "drepturi de acces user: %s\n", user_permissions);
    dprintf(stats_fd, "drepturi de acces grup: R--\n");
    dprintf(stats_fd, "drepturi de acces altii: ---\n");
 

    close(stats_fd);
    exit(EXIT_SUCCESS);

	  
	}}}
  


    //asteptare fii
    int status;
    while (waitpid(-1, &status, 0) > 0) {
        if (WIFEXITED(status)) {
            printf("S-a încheiat procesul cu pid-ul %d și codul %d\n", getpid(), WEXITSTATUS(status));
        }
    }

    closedir(dir);
    
    return 0;
}
