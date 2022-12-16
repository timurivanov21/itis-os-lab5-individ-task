#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/wait.h>

int iter_directory(char *path1, char *path2, int N) {
  DIR *dir1;
  DIR *dir2;

  FILE *file1;
  FILE *file2;

  char c1;
  char c2;

  struct stat buf1;
  struct stat buf2;

  struct dirent *dir1_info;
  struct dirent *dir2_info;

  int bytes_counter = 0;
  bool same_flag;
  int fork_counter = 0;
  int status;
  pid_t ppid;

  if((dir1=opendir(path1))==NULL) {
    printf("Error: OpenDirError.\n");
    return 1;
  }

  else {
    dir1_info = readdir(dir1);
    while(dir1_info!=NULL) {

      char local_path1[strlen(path1) + strlen(dir1_info->d_name) + 1];
      sprintf(local_path1, "%s%s%s", path1, "/", dir1_info->d_name);
      lstat(local_path1, &buf1);

      if(S_ISREG(buf1.st_mode)) {

        if((dir2=opendir(path2))==NULL) {
          printf("Error: OpenDirError.\n");
          return 1;
        }

        dir2_info = readdir(dir2);
        while(dir2_info!=NULL) {

          if (fork_counter == N) {
            while((ppid = wait(&status)) > 0);
            printf("\n\n -----%d----- \n\n", fork_counter);
            fork_counter = 0;
          }

          char local_path2[strlen(path2) + strlen(dir2_info->d_name) + 1];
          sprintf(local_path2, "%s%s%s", path2, "/", dir2_info->d_name);
          lstat(local_path2, &buf2);

          if(S_ISREG(buf2.st_mode)) {

            pid_t pid = fork();
            fork_counter ++;

            if (pid==0) {

              if((file2=fopen(local_path2, "r"))==NULL || (file1=fopen(local_path1, "r"))==NULL) {
                printf("Error: OpenFileError.\n");
                dir2_info = readdir(dir2);
                continue;
              }

              c1 = fgetc(file1);
              c2 = fgetc(file2);
              bytes_counter = 2;
              same_flag = false;

              while(c1 == c2) {
                if(c1 == EOF) {
                  same_flag = true;
                  break;
                }
                c1 = fgetc(file1);
                c2 = fgetc(file2);
                bytes_counter = bytes_counter + 2;
              }

              if(fclose(file2)!=0 || fclose(file1)!=0) {
                printf("Error:FileCloseError.\n");
                dir2_info = readdir(dir2);
                continue;
              }

              printf("%d | %s | %s | %d | %d\n", getpid(), dir1_info->d_name, dir2_info->d_name, bytes_counter, same_flag);
              return 0;
            }
          }
          dir2_info = readdir(dir2);
        }
        if(closedir(dir2)!=0) {
          printf("DirCloseError.\n");
          return 1;
        }
      }

      dir1_info = readdir(dir1);
    }
    if(closedir(dir1)!=0) {
      printf("DirCloseError.\n");
      return 1;
    }
  }
  while((ppid = wait(&status)) > 0);
  return 0;
}

int main(int argc, char *argv[]) {
  int result = 0;
  char *path1 = argv[1];
  char *path2 = argv[2];
  int N = atoi(argv[3]);

  result = iter_directory(path1, path2, N);

  return result;
}
