#include <stdio.h>  
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>


int main(int argc, char **argv)
{

struct stat st;


if(argv[2]!=NULL)
{
printf("”Usage ./program ”.\n");
exit(-1);
}


char *fin=argv[1];


int img=open(fin,O_RDONLY);
if(img==-1)
{
perror("Imaginea nu poate fi accesata\n");
exit(-1);
}

if(fstat(img,&st)==-1)
{
perror("”Usage ./program ”.\n");
exit(-1);
}





int fout=open("statistica.txt",O_CREAT,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);

if(fout==-1)
{
perror("Fisierul destinatie nu a putut fi deschis\n");
exit(-1);
}


char *filename=argv[1];


//inaltime&lungime
char* buffer=malloc(32*sizeof(char));

read(img,buffer,18);
read(img,buffer,4);

char* Width=malloc(4*sizeof(char));
strcpy(Width,buffer);
read(img,buffer,4);

char* Height=malloc(4*sizeof(char));

strcpy(Height,buffer);
strcat(Width," ");
strcat(Width,Height);

int dimensiune;
//int time;
int user_id;

dimensiune=st.st_size;
//time=st.st_atim;
user_id=st.st_uid;

printf("dimensiune fisier: %d bytes\n, nume fisier: %s\n, userid: %d \n inaltime:%s\n latime: %s\n ", dimensiune,filename,user_id,Height,Width);

char statistics[1000];

sprintf(statistics, " dimensiune fisier: %d bytes\n, nume fisier: %s\n, userid: %d", dimensiune,filename,user_id);




if (write(fout, statistics, strlen(statistics)) == -1)
perror("Eroare la scrierea fisierului de statistica");

return 0;

}