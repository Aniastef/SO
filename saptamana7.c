#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
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

void process_file(const char *filename, int stats_fd) {
  
  //deschidre bmp
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("Eroare la deschiderea fisierului");
        return;
    }

    struct BMPHeader bmp_header;
    
    //citire header
    if (read(fd, &bmp_header, sizeof(bmp_header)) != sizeof(bmp_header)) {
        perror("Eroare la citirea header-ului BMP");
        close(fd);
        return;
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
    dprintf(stats_fd, "nume fisier: %s\n", filename);
    dprintf(stats_fd, "inaltime: %d\n", height);
    dprintf(stats_fd, "lungime: %d\n", width);
    dprintf(stats_fd, "dimensiune: %d\n", file_size);
    dprintf(stats_fd, "identificatorul utilizatorului: %d\n", user_id);
    dprintf(stats_fd, "timpul ultimei modificari: %s\n", time_str);
    dprintf(stats_fd, "contorul de legaturi: %ld\n", (long)file_size); 
    dprintf(stats_fd, "drepturi de acces user: %s\n", user_permissions);
    dprintf(stats_fd, "drepturi de acces grup: R--\n");
    dprintf(stats_fd, "drepturi de acces altii: ---\n");
}



void process_directory(const char *dirname, int stats_fd) {
    struct stat dir_stat;
    if (stat(dirname, &dir_stat) == -1) {
        perror("Eroare la obtinerea informatiilor despre director");
        return;
    }

    // Informatii director
    dprintf(stats_fd, "nume director: %s\n", dirname);
    dprintf(stats_fd, "identificatorul utilizatorului: %d\n", getuid());
    dprintf(stats_fd, "drepturi de acces user: RWX\n");
    dprintf(stats_fd, "drepturi de acces grup: R--\n");
    dprintf(stats_fd, "drepturi de acces altii: ---\n");

    // Parcurgere director
    DIR *dir = opendir(dirname);
    if (dir == NULL) {
        perror("Eroare deschidere director");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        char path[PATH_MAX];
        
	snprintf(path, PATH_MAX, "%s/%s", dirname, entry->d_name);

        if (entry->d_type == DT_REG && strstr(entry->d_name, ".bmp") != NULL)
	{
	  // .bmp
            process_file(path, stats_fd);
        }
	
	else if (entry->d_type == DT_REG)
	  
	 {
            //fisier care nu e .bmp
            process_file(path, stats_fd);
        }
	
	else if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
	 
 
    }

    closedir(dir);
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        perror("Eroare numar de argumente");
        exit(EXIT_FAILURE);
    }

    char *input_path = argv[1];

    //deschidere statistica
    int stats_fd = open("statistica.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (stats_fd == -1) {
        perror("Eroare la crearea fisierului de statistici");
        exit(EXIT_FAILURE);
    }

    //procesare input
    struct stat input_stat;
    if (stat(input_path, &input_stat) == -1) {
        perror("Eroare la obtinerea informatiilor despre input");
        close(stats_fd);
        exit(EXIT_FAILURE);
    }

    if (S_ISREG(input_stat.st_mode)) {
      //daca e fisier normal
        process_file(input_path, stats_fd);
    } else if (S_ISDIR(input_stat.st_mode)) {
        //daca e director
        process_directory(input_path, stats_fd);
    } else {
        perror("Eroare input: nu e fisier normal nici director");
    }


    close(stats_fd);

    return 0;
}
