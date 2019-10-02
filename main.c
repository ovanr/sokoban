#include <stdio.h>
#include <stdlib.h>
#include "queue.h"
#include <math.h>
#include <string.h>
#include <float.h>

#define True 1
#define False 0

// TODO : replace state with higher path cost, with the identical with lower path cost (guaranteed that freeing old state won't interfere with other states)
//                to do this duplicates check shouldn't check in a parent upwards method but old notes should be stored in a queue) 

typedef struct {
   u_int x;
   u_int y;
} Position;

typedef struct state {
   u_char move_from_parent;
   struct state *parent;
   Position *current_pos;
   Position *boxes;
   int cost_score;
   double heuristic_score;
} State;
   
void err_exit(char *msg) {
   fprintf(stderr, "%s\n", msg);
   exit(1);
}

// calculate euclidian distance between boxes and goal positions in a fifo priority
// manner, so the first box in boxes array chooses from whole array, then the second
// box chooses from the remaining N-1, and so on.
// **boxes = box[0] = x-coordinates, box[1] = y-coordinates
// same with goal_positions
double calculate_heuristic_score(Position *boxes, Position *goal_positions, u_int num_boxes) {
   double total_score = 0;
   u_char used_goal_positions[num_boxes];
   for (u_int x =0; x < num_boxes; x++)
      used_goal_positions[x] = 0;
   
   for (u_int i = 0; i < num_boxes; i++) {
      double min_score = DBL_MAX;
      u_int min_box = 0;
      for (u_int c = 0; c < num_boxes; c++) {
         if (used_goal_positions[c] == 1)
            continue;
         
         // get score for box i and pos i
         double score = sqrt(pow((double) boxes[i].x-goal_positions[c].x,2) + \
                              pow((double) boxes[i].y-goal_positions[c].y,2));
         
         
         if (min_score > score) {
            min_score = score;
            min_box = c;
         }
      }
      used_goal_positions[min_box] = 1;
      total_score += min_score;
   }
   
   return total_score;
}

int compare_state(void *present_state, void *new_state) {
   State *ps = present_state;
   State *ns = new_state;
   double p_score = ps->heuristic_score + ps->cost_score;
   double n_score = ns->heuristic_score + ns->cost_score;
   
   if (p_score > n_score)
      return 1;
   if (p_score < n_score)
      return -1;
   
   return 0;
}

_Bool weak_deadlock_detection(char **puzzle, Position *boxes, Position *goal_positions, int num_boxes) {
   _Bool no_box_moves = True;
   _Bool all_goals_satisfied = True;
   
   for (int gl = 0; gl < num_boxes; gl++)
      if (puzzle[goal_positions[gl].x][goal_positions[gl].y] <= 0) {
         all_goals_satisfied = False;
         break;
      }
   if (all_goals_satisfied)
      return False;
   
   for (int bx = 0; bx < num_boxes; bx++) {
      _Bool x_lock = False;
      _Bool y_lock = False;
      u_char x = boxes[bx].x;
      u_char y = boxes[bx].y;
      
      _Bool sits_on_goal = False;
      for (int gl = 0; gl < num_boxes; gl++)
         if ((goal_positions[gl].x == x) && (goal_positions[gl].y == y)) {
            sits_on_goal = True;
            break;
         }
      if (sits_on_goal)
         continue;
      
      // check if box is permanently stuck (wall check only)
      if ((puzzle[x-1][y] == -1) || (puzzle[x+1][y] == -1))
         x_lock = True;
      if ((puzzle[x][y-1] == -1) || (puzzle[x][y+1] == -1))
         y_lock = True;
      if (x_lock && y_lock)
         return True;
      
      if ((puzzle[x-1][y] + puzzle[x+1][y]) == 0)
         no_box_moves = False;
      if ((puzzle[x][y-1] + puzzle[x][y+1]) == 0)
         no_box_moves = False;
   }
   
   return no_box_moves;
}

_Bool check_duplicates(State *state, Position *new_cursor_pos, Position *new_boxes, int num_boxes) {
   if (state == NULL) 
      return False;
      
   if ((state->current_pos->x != new_cursor_pos->x) ||
      (state->current_pos->y != new_cursor_pos->y))
      return check_duplicates(state->parent, new_cursor_pos, new_boxes, num_boxes);
   

   for (int i = 0; i < num_boxes; i++) {
      _Bool found_identical_box = False;
      for (int j = 0; j < num_boxes; j++)
         if ((state->boxes[i].x == new_boxes[j].x) &&
            (state->boxes[i].y == new_boxes[j].y)) {
            found_identical_box = True;
            break;
         }
      if (found_identical_box == False)
         return check_duplicates(state->parent, new_cursor_pos, new_boxes, num_boxes);
   }
   return True;
}

void print_solution(char **puzzle_template, State *sol, Position *goal_positions, int num_boxes, int width, int height) {
   printf("++++++++++++++++++++\n");
   char puzzle_cpy[height][width];
   for (int i = 0; i < height; i++) 
      for (int j = 0; j < width; j++) 
         puzzle_cpy[i][j] = puzzle_template[i][j];
      
   for (int i = 0; i < num_boxes; i++) {
      puzzle_cpy[sol->boxes[i].x][sol->boxes[i].y] = 1;
      printf("Box %d coordinates:   %d %d\n", i, sol->boxes[i].x, sol->boxes[i].y);
   }
   
   for (int i = 0; i < num_boxes; i++) {
      printf("Goal %d coordinates: %d %d\n",i,  goal_positions[i].x, goal_positions[i].y);
   }
   puzzle_cpy[sol->current_pos->x][sol->current_pos->y] = -19;
   printf("move score:    %d\n", sol->cost_score);
   printf("heuristic:    %f\n", calculate_heuristic_score(sol->boxes, goal_positions, num_boxes));
   printf("\n");
   for (int i = 0; i < height; i++) {
      for (int j = 0; j < width; j++) 
         switch (puzzle_cpy[i][j]) {
            case (1): printf("B"); break;
            case (0): printf(" "); break;
            case (-19): printf("^"); break;
            case (-1): printf("#"); break;
            default: printf("?"); break;
         }
         
         printf("\n");
   }
   printf("-----------------\n");
   
}

void getSolution(State *sol) {
   if (sol->parent == NULL)
      return;
   switch (sol->move_from_parent) {
      case (1): printf("up ");break;
      case (2): printf("down ");break;
      case (3): printf("left ");break;
      case (4): printf("right ");break;
   }
   getSolution(sol->parent);
}   
   
int make_move(Queue *states, char **puzzle, Position *goal_positions, State *root_state, u_int num_boxes) {  
   
   // set boxes to graph
   for (u_int i = 0 ; i < num_boxes; i++)
      puzzle[root_state->boxes[i].x][root_state->boxes[i].y] = (i+1);
   
   Position *cur_pos = root_state->current_pos;
   
   char x_offsets[] = { -1 , 1, 0, 0 };
   char y_offsets[] = { 0, 0, -1, 1 };
   
   for (int mv = 0; mv < 4; mv++) {
      _Bool valid = True;
      int new_x = cur_pos->x + x_offsets[mv];
      int new_y = cur_pos->y + y_offsets[mv];
      Position *new_pos = malloc(sizeof(Position));
      new_pos->x = new_x;
      new_pos->y = new_y;
      
      // if it's a wall
      if (puzzle[ new_x ][ new_y ] < 0) 
         valid = False;
      
      // if it's a box
      if ((puzzle[ new_x ][ new_y ] > 0) && 
         (puzzle[ new_x + x_offsets[mv] ][ new_y + y_offsets[mv] ] != 0)) 
         // no space for box to move upwards
         valid = False;

      if ((valid) && (puzzle[ new_x ][ new_y ] != 0)) { // if a box moved
         
         // create new state of puzzle && and check for deadlocks
         u_int box_id = puzzle[new_x][new_y] - 1;
         puzzle[ new_x + x_offsets[mv] ][ new_y + y_offsets[mv] ] = puzzle[ new_x ][ new_y ];
         puzzle[ new_x][ new_y ] = 0;
         
         root_state->boxes[box_id].x = new_x + x_offsets[mv];
         root_state->boxes[box_id].y = new_y + y_offsets[mv];
         
         if (weak_deadlock_detection(puzzle, root_state->boxes, goal_positions, num_boxes)) // is deadlock detected ?
            valid = False;
         if (check_duplicates(root_state->parent, new_pos, root_state->boxes, num_boxes)) 
            valid = False;
            
         // revert changes in puzzle
         puzzle[ new_x ][ new_y ] = puzzle[ new_x  + x_offsets[mv] ][ new_y + y_offsets[mv]];
         puzzle[ new_x + x_offsets[mv] ][ new_y + y_offsets[mv] ] = 0;
         root_state->boxes[box_id].x = new_x;
         root_state->boxes[box_id].y = new_y;
      }

      
      if ((valid) && (puzzle[ new_x ][ new_y ] == 0)) // if only cursor move
         if (check_duplicates(root_state->parent, new_pos, root_state->boxes, num_boxes)) 
            valid = False;
         
      if (valid) {
         Position *new_pos = malloc(sizeof(Position));
         Position *new_boxes = malloc(sizeof(Position)*num_boxes);
         State *new_state = malloc(sizeof(State));
         memcpy(new_boxes, root_state->boxes, sizeof(Position)*num_boxes);
            
         if (puzzle[new_x][new_y] != 0) {
            u_int box_id = puzzle[new_x][new_y] - 1;
            new_boxes[box_id].x = new_x + x_offsets[mv];
            new_boxes[box_id].y = new_y + y_offsets[mv];
         }
         new_pos->x = new_x;
         new_pos->y = new_y;
         new_state->move_from_parent = mv+1;
         new_state->parent = root_state;
         new_state->current_pos = new_pos;
         new_state->boxes = new_boxes;
         new_state->cost_score = root_state->cost_score+1;
         new_state->heuristic_score = calculate_heuristic_score(new_boxes, goal_positions, num_boxes);
         

         //print_solution(puzzle, new_state, goal_positions, num_boxes, 7);
            
            
         insert_sorted_queue(states, new_state, compare_state);
      }
   }
   
   // unset boxes to graph
   for (u_int i = 0 ; i < num_boxes; i++)
      puzzle[root_state->boxes[i].x][root_state->boxes[i].y] = 0;
   
   return 0;   
}

State *search_solution(char **puzzle_template, Queue *states, Position *goal_positions, u_int num_boxes) {
   int nodes = 1;
   while (states->length > 0) {
      State *state = remove_head_queue(states);
      
      if (state->heuristic_score == 0.0) {
         printf("Found after %d nodes\n", nodes);
         return state;
      }
      make_move(states, puzzle_template, goal_positions, state, num_boxes);
      nodes++;
   }
   
   
   return NULL;
}

int main(void) {   

   char temp[20];
      
   if (fgets(temp, 20, stdin) == NULL)
      err_exit("Incorrect format of file");
      
   u_int puzzle_size = strtol(temp, NULL, 10);
   
   char line[200];
   char **puzzle_template= malloc(sizeof(char *) * puzzle_size);
   
   if (puzzle_template== NULL)
      err_exit("Memory Error");
   
   Position current_pos;
   u_int line_number = 0;
   u_int width = 0;
   u_int boxes_id = 0;
   u_int goal_pos_id = 0;
   Position *boxes = malloc(sizeof(Position)*puzzle_size*puzzle_size);
   Position *goal_positions = malloc(sizeof(Position)*puzzle_size*puzzle_size);
   
   while (fgets(line, 200, stdin) != NULL) {
      printf("DEBUG->%s", line);
      
      if (line_number >= puzzle_size) {
         err_exit("Found too many lines while reading puzzle\n");
      }
      width = strlen(line);
      puzzle_template[line_number] = malloc(sizeof(char)*width);
      if (puzzle_template[line_number] == NULL)
         err_exit("Memory Error");
      
      u_int i = 0;
      for (char *ptr = line; *ptr; ptr++) {
         puzzle_template[line_number][i] = 0;

         switch (*ptr) {
            case '#':
               puzzle_template[line_number][i] = -1;
               break;
            case ' ':
               break;
            case '*':
               goal_positions[goal_pos_id].x = line_number;
               goal_positions[goal_pos_id].y = i;
               goal_pos_id++;
            case '$':
               boxes[boxes_id].x = line_number;
               boxes[boxes_id].y = i;
               boxes_id++;
               break;
            case '+':
               current_pos.x = line_number;
               current_pos.y = i;
            case '.':
               goal_positions[goal_pos_id].x = line_number;
               goal_positions[goal_pos_id].y = i;
               goal_pos_id++;
               break;              
            case '@':
               current_pos.x = line_number;
               current_pos.y = i;
               break;
            case '\n':
               break;
               
            default:
               free(puzzle_template);
               free(boxes);
               free(goal_positions);
               char err_msg[30];
               sprintf(err_msg, "Unknown character: %c\n", *ptr); 
               err_exit(err_msg);
               break;
         }
         i++;
      }
      line_number++;
   }
   
   boxes = realloc(boxes, boxes_id*sizeof(Position));
   goal_positions = realloc(goal_positions, goal_pos_id*sizeof(Position));
   
   State *root_state = malloc(sizeof(State));
   root_state->parent = NULL;
   root_state->move_from_parent = 0;
   root_state->boxes = boxes;
   root_state->current_pos = &current_pos;
   root_state->cost_score = 0;
   root_state->heuristic_score = calculate_heuristic_score(root_state->boxes, goal_positions, boxes_id);
   print_solution(puzzle_template, root_state, goal_positions, boxes_id, width, line_number);
   Queue *states;
   init_queue(&states);
   insert_head_queue(states, root_state);
   State *solution;
   
   if (NULL != (solution = search_solution(puzzle_template, states, goal_positions, boxes_id))) {
      print_solution(puzzle_template, solution,goal_positions, boxes_id, width, line_number);
      getSolution(solution);
      return 0;
   } else
      return 1;
}
   
