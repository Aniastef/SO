#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

struct BMPHeader {
    int file_size;
    short reserved1;
    short reserved2;
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

int main(int argc, char *argv[]) {
  
  //nr de argumente
   if (argc != 2) {
     perror("Eroare numar de argumente");
     exit(EXIT_FAILURE);
}

    char *input_file = argv[1];

    //deschidere bmp
    int fd = open(input_file, O_RDONLY);
    if (fd == -1) {
        perror("Eroare la deschiderea fisierului");
        exit(EXIT_FAILURE);
    }

    //citire header 
    struct BMPHeader bmp_header;
    if (read(fd, &bmp_header, sizeof(bmp_header)) != sizeof(bmp_header)) {
        perror("Eroare la citirea header-ului BMP");
        close(fd);
        exit(EXIT_FAILURE);
    }

    //informatii header
    int height = bmp_header.height;
    int width = bmp_header.width;
    int file_size = bmp_header.file_size;
    int user_id = getuid();

    // timp
    time_t modification_time = bmp_header.data_offset; 
    struct tm *modification_tm = localtime(&modification_time);
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%d.%m.%Y", modification_tm);

    close(fd);

    // creare fisier statistica.txt
    int stats_fd = open("statistica.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (stats_fd == -1) {
        perror("Eroare la crearea fisierului de statistici");
        exit(EXIT_FAILURE);
    }

    char user_permissions[10];
    get_permissions(S_IRUSR | S_IWUSR | S_IXUSR, user_permissions);
    
    //scriere informatii in staatistica
    dprintf(stats_fd, "nume fisier: %s\n", input_file);
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

    return 0;
}
