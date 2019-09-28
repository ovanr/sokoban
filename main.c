#include <stdio.h>
#include <stdlib.h>

typedef unsigned char u_char;
typedef unsigned int u_int;

void err_exit(char *msg) {
   fprintf(stderr, "%s\n", msg);
   exit(1);
}

int main(void) {   

   char temp[20];
      
   if (fgets(temp, 20, stdin) == NULL)
      err_exit("Incorrect format of file");
      
   u_int puzzle_size = strtol(temp, NULL, 10);
   
   char line[puzzle_size+1];
   u_char **puzzle = malloc(sizeof(char *) * puzzle_size);
   
   if (puzzle == NULL)
      err_exit("Memory Error");
   
   u_int line_number = 0;
   while (fgets(line, puzzle_size+1, stdin) != NULL) {
      puzzle[line_number] = malloc(sizeof(char)*strlen(line));
      if puzzle[line_number] == NULL)
         err_exit("Memory Error");
      
      
      for (char *ptr = line; *ptr; ptr++) {
         switch (*ptr) {
            case '#':
            case ' ':
               puzzle[line_number][i] = *ptr;
               break;
            case '$':
               
            case '.':
               
            case '*':
               
            case '@':
               
            case '+':
               
            default:
               free(puzzle);
               char err_msg[30];
               sprintf(error_msg, "Unknown character: %c\n", *ptr) 
               err_exit(err_msg);
               break;
               
   printf("%d\n", puzzle_size);
   return 0;
}
   
