#include <stdio.h>  
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

int main (int argc, char **argv)
{
  int file = open (argv[1], O_RDONLY);
  int ofile = open ("output2.txt", O_RDONLY | O_WRONLY | O_CREAT | O_TRUNC, 0644);
  struct stat buf;
  fstat (file, &buf);
  char sbuf[100];
  uint32_t rez[2];
  lseek(file, 18, SEEK_CUR);
  
  read(file,rez,8);

  sprintf(sbuf,"rezolution is %dx%d\n",rez[0],rez[1]);
  write (ofile, sbuf, strlen (sbuf));

  sprintf (sbuf, "file total size is: %ld\n", buf.st_size);
  write (ofile, sbuf, strlen (sbuf));
  sprintf (sbuf, "number of hard links: %ld\n", buf.st_nlink);
  write (ofile, sbuf, strlen (sbuf));
  sprintf (sbuf, "User ID of Owner: %ld\n", buf.st_uid);
  write (ofile, sbuf, strlen (sbuf));
  sprintf (sbuf, "drepturi de access user ");
  strcat (sbuf, ((buf.st_mode & S_IRUSR) ? "R" : "-"));
  strcat (sbuf, ((buf.st_mode & S_IWUSR) ? "W" : "-"));
  strcat (sbuf, ((buf.st_mode & S_IXUSR) ? "X\n" : "-\n"));
  write (ofile, sbuf, strlen (sbuf));
  sprintf (sbuf, "drepturi de access group ");
  strcat (sbuf, ((buf.st_mode & S_IRGRP) ? "R" : "-"));
  strcat (sbuf, ((buf.st_mode & S_IWGRP) ? "W" : "-"));
  strcat (sbuf, ((buf.st_mode & S_IXGRP) ? "X\n" : "-\n"));
  write (ofile, sbuf, strlen (sbuf));
  sprintf (sbuf, "drepturi de access other ");
  strcat (sbuf, ((buf.st_mode & S_IROTH) ? "R" : "-"));
  strcat (sbuf, ((buf.st_mode & S_IWOTH) ? "W" : "-"));
  strcat (sbuf, ((buf.st_mode & S_IXOTH) ? "X\n" : "-\n"));
  write (ofile, sbuf, strlen (sbuf));

  close (file);
  close (ofile);
  return 0;

}