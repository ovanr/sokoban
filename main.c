#include <stdio.h>
#include <stdlib.h>
#include "queue.h"
#include <math.h>
#include <string.h>
#include <float.h>

#define True 1
#define False 0

#define PUZZLE_WIDTH_LIMIT 200
#define OPTIMALITY_STRICTNESS 2  // 1 is for optimal, higher values sacrifice optimality for speed

typedef struct {
   u_int x;
   u_int y;
} Coordinate;

typedef struct state {
   u_char move_from_parent;
   struct state *parent;
   Coordinate *current_pos;
   Coordinate *boxes;
   int cost_score;
   double heuristic_score;
} State;
   
void err_exit(char *msg) {
   fprintf(stderr, "%s\n", msg);
   exit(1);
}

// (A) find the box with the lowest euclidian distance from goal position
// (B) and for the rest boxes that aren't in goal position add a penalty per box given by OPTIMALITY_STRICTNESS.
// for the heuristic to be fully optimal, the OPTIMALITY_STRICTNESS assumes you will only need one move
// per box to move to goal position. you can change this to something more reasonable but optimality may be sacrificed.
// the euclidian distance of the current cursor compared to the box in (A) is added to the final score as well.
double calculate_heuristic_score(Coordinate *boxes, Coordinate *goal_positions, Coordinate *cursor, u_int num_boxes) {
   double total_score = 0;
   int mismatched_boxes_count = num_boxes;
   double cursor_closest = 0;
   
   double global_min_score = DBL_MAX;
   
   for (u_int i = 0; i < num_boxes; i++) {
      double local_min_score = DBL_MAX;
      double score;
      for (u_int c = 0; c < num_boxes; c++) {
         
         // get score for box i and pos i
         score = sqrt(pow((double) boxes[i].x-goal_positions[c].x,2) + \
         pow((double) boxes[i].y-goal_positions[c].y,2));
         if (score == 0)
            break;
         
         if (local_min_score > score) 
            local_min_score = score;
         
      }
      if (score == 0) // if the box isn't in goal position then we can count its minimum score
         mismatched_boxes_count--;
      else if (global_min_score > local_min_score) {
         cursor_closest =  sqrt(pow((double) boxes[i].x-cursor->x,2) + \
         pow((double) boxes[i].y-cursor->y,2));
         global_min_score = local_min_score;
      }
   }
   total_score = (mismatched_boxes_count == 0) ? 0 : (mismatched_boxes_count*OPTIMALITY_STRICTNESS)+global_min_score;
   return total_score + cursor_closest;
}

// calculate euclidian distance between boxes and goal positions in a fifo priority
// manner, so the first box in boxes array chooses from whole array, then the second
// box chooses from the remaining N-1, and so on. 
// NON-OPTIMAL
// the minimum euclidian distance of the current cursor compared to all the boxes is added to the final score as well.
double calculate_heuristic_score2(Coordinate *boxes, Coordinate *goal_positions, Coordinate *cursor, u_int num_boxes) {
   double total_score = 0;
   u_char used_goal_positions[num_boxes];
   double cursor_closest = DBL_MAX;
   
   for (u_int x =0; x < num_boxes; x++)
      used_goal_positions[x] = 0;
   
   for (u_int i = 0; i < num_boxes; i++) {
      double temp =  sqrt(pow((double) boxes[i].x-cursor->x,2) + \
      pow((double) boxes[i].y-cursor->y,2));
      if (temp < cursor_closest)
         cursor_closest = temp;
      
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
   
   return total_score + cursor_closest;
}

// calculate the euclidian distance between the boxes and all the goal positions 
// and add the minimum distance per box to final score.
// OPTIMAL
// the minimum euclidian distance of the current cursor compared to all the boxes is added to the final score as well.
double calculate_heuristic_score1(Coordinate *boxes, Coordinate *goal_positions, Coordinate *cursor, u_int num_boxes) {
   double total_score = 0;   
   double cursor_closest = DBL_MAX;
   
   for (u_int i = 0; i < num_boxes; i++) {
      double temp =  sqrt(pow((double) boxes[i].x-cursor->x,2) + \
            pow((double) boxes[i].y-cursor->y,2));
      if (temp < cursor_closest)
         cursor_closest = temp;
      
      double local_min_score = DBL_MAX;
      for (u_int c = 0; c < num_boxes; c++) {
         
         // get score for box i and pos i
         double score = sqrt(pow((double) boxes[i].x-goal_positions[c].x,2) + \
                              pow((double) boxes[i].y-goal_positions[c].y,2));
         
         if (score < local_min_score) {
            local_min_score = score;
         }
      }
      total_score += local_min_score;
   }
   
   total_score += cursor_closest;
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

//deadlock -> if wall above or below -> if (no goal on same line) AND no space on wall row -> deadlock
//            if wall left or right -> if (no goal on same column) AND no space on wall column-> deadlock
_Bool simple_deadlock_detect(char **puzzle, Coordinate *boxes, int num_boxes) {
   
   for (int bx = 0; bx < num_boxes; bx++) {
      u_char x = boxes[bx].x;
      u_char y = boxes[bx].y;

      if (puzzle[x][y] == '.') 
         continue;
      
      _Bool x_lock = False;
      _Bool y_lock = False;
      
      // check if box is permanently stuck (wall check only)
      if (puzzle[x-1][y] == '#' || (puzzle[x+1][y] == '#'))  
         x_lock = True;
         
      if ((puzzle[x][y-1] == '#') || (puzzle[x][y+1] == '#'))
         y_lock = True;
      
      if (x_lock && y_lock)
         return True;
         
      int ptr = y;
      for (; puzzle[x][ptr] != '#'; ptr--) ;
      ptr++;
      
      _Bool way_out = False;
      for (; puzzle[x][ptr] != '#'; ptr++) 
         if ((puzzle[x][ptr] == '.') || \
            ((puzzle[x-1][ptr] != '#') && (puzzle[x+1][ptr] != '#'))) {
            way_out = True;
            break;
         }
      if (! way_out) 
         return True;
      
      ptr = x;
      for (; puzzle[ptr][y] != '#'; ptr--) ;
      ptr++;
      
      way_out = False;
      for (; puzzle[ptr][y] != '#'; ptr++) 
         if ((puzzle[ptr][y] == '.') || \
            ((puzzle[ptr][y-1] != '#') && (puzzle[ptr][y+1] != '#'))) {
            way_out = True;
            break;
         }
            
      if (! way_out) 
         return True;
      
   }
   
   return False;
}

void print_state(char **puzzle, State *sol, Coordinate *goal_positions, int num_boxes, int width, int height) {
   printf("++++++++++++++++++++\n");
   char puzzle_cpy[height][width];
   for (int i = 0; i < height; i++) 
      for (int j = 0; j < width; j++) 
         puzzle_cpy[i][j] = puzzle[i][j];
      
   for (int i = 0; i < num_boxes; i++) {
      puzzle_cpy[sol->boxes[i].x][sol->boxes[i].y] = ( puzzle_cpy[sol->boxes[i].x][sol->boxes[i].y] == '.') ? '*' : '$';
      printf("Box %d coordinates:   %d %d\n", i, sol->boxes[i].x, sol->boxes[i].y);
   }
   
   for (int i = 0; i < num_boxes; i++) {
      printf("Goal %d coordinates: %d %d\n",i,  goal_positions[i].x, goal_positions[i].y);
   }

   printf("move score:    %d\n", sol->cost_score);
   printf("heuristic:    %f\n", sol->heuristic_score);
   printf("\n");
   puzzle_cpy[sol->current_pos->x][sol->current_pos->y] = ( puzzle_cpy[sol->current_pos->x][sol->current_pos->y] == '.') ? '+' : '@';
   for (int i = 0; i < height; i++) {
      for (int j = 0; j < width; j++) 
         printf("%c", puzzle_cpy[i][j]);
         
      printf("\n");
   }
   printf("-----------------\n");
      
}

State *get_duplicate(Queue *history, Coordinate *new_cursor_pos, Coordinate *new_boxes, int num_boxes) {

   Node *ptr = history->head;
   while (ptr != NULL) {
      State *state = ptr->data;
      if ((state->current_pos->x != new_cursor_pos->x) ||
         (state->current_pos->y != new_cursor_pos->y)) {
         ptr = ptr->next;
         continue;
      }
      _Bool identical_states = True;
      for (int i = 0; i < num_boxes; i++) {
         _Bool identical_boxes = False;
         for (int j = 0; j < num_boxes; j++)
            if ((state->boxes[i].x == new_boxes[j].x) &&
               (state->boxes[i].y == new_boxes[j].y)) {
               identical_boxes = True;
               break;
            }
         if (identical_boxes == False) {
            ptr = ptr->next;
            identical_states = False;
            break;
         }
      }
      if (identical_states)
         return state;
      
   }
   return NULL;

}

void getSolution(State *sol) {
   if (sol->parent == NULL)
      return;
   switch (sol->move_from_parent) {
      case (0): printf("up ");break;
      case (1): printf("down ");break;
      case (2): printf("left ");break;
      case (3): printf("right ");break;
   }
   getSolution(sol->parent);
}   
   
int make_move(char **puzzle, char **puzzle_temp, Queue *states, Queue *history, State *current_state, Coordinate *goal_positions, u_int num_boxes) {  
   
   // set boxes to graph
   for (u_int i = 0 ; i < num_boxes; i++) 
      puzzle_temp[current_state->boxes[i].x][current_state->boxes[i].y] = (i+1);
   
   Coordinate *cur_pos = current_state->current_pos;
   
   char x_offsets[] = { -1 , 1, 0, 0 };
   char y_offsets[] = { 0, 0, -1, 1 };
   
   for (int mv = 0; mv < 4; mv++) {
      int new_x = cur_pos->x + x_offsets[mv];
      int new_y = cur_pos->y + y_offsets[mv];
      
      // if it's a wall
      if (puzzle[ new_x ][ new_y ] == '#') 
         continue;
      
      if ((puzzle_temp[ new_x ][ new_y ] > 0) &&  // if it's a box
         ((puzzle[ new_x + x_offsets[mv] ][ new_y + y_offsets[mv] ] == '#') ||
         (puzzle_temp[ new_x + x_offsets[mv] ][ new_y + y_offsets[mv] ] > 0)))
         // no space for box to move upwards
         continue;

      Coordinate new_pos = { new_x, new_y };
      
      _Bool valid = True;
      u_int box_id = puzzle_temp[new_x][new_y] - 1;
      
      if (puzzle_temp[ new_x ][ new_y ] != 0) { // if a box moved
         
         current_state->boxes[box_id].x = new_x + x_offsets[mv];
         current_state->boxes[box_id].y = new_y + y_offsets[mv];
         
         if (simple_deadlock_detect(puzzle, current_state->boxes, num_boxes)) // is deadlock detected ?
             valid = False;
         
      }

      State *identical = NULL;
      if (valid && (NULL != (identical = get_duplicate(history, &new_pos, current_state->boxes, num_boxes))))
         valid = False;
      if (valid && (NULL != (identical = get_duplicate(states, &new_pos, current_state->boxes, num_boxes))))
         valid = False;
      
      if ((identical != NULL) && (identical->cost_score > current_state->cost_score+1)) { // lower cost state substitution
         identical->parent = current_state->parent;
         identical->move_from_parent = mv;
         identical->cost_score = current_state->cost_score+1;
      }
      
      if (puzzle_temp[new_x][new_y] != 0) {
         // revert changes in box
         current_state->boxes[box_id].x = new_x;
         current_state->boxes[box_id].y = new_y;
      }
      
      if (!valid)
         continue;
      

      Coordinate *cursor = malloc(sizeof(Coordinate));
      Coordinate *new_boxes = malloc(sizeof(Coordinate)*num_boxes);
      State *new_state = malloc(sizeof(State));
      memcpy(new_boxes, current_state->boxes, sizeof(Coordinate)*num_boxes);
         
      if (puzzle_temp[new_x][new_y] != 0) {
         new_boxes[box_id].x = new_x + x_offsets[mv];
         new_boxes[box_id].y = new_y + y_offsets[mv];
      }
      cursor->x = new_x;
      cursor->y = new_y;
      new_state->move_from_parent = mv;
      new_state->parent = current_state;
      new_state->current_pos = cursor;
      new_state->boxes = new_boxes;
      new_state->cost_score = current_state->cost_score+1;
      new_state->heuristic_score = calculate_heuristic_score(new_boxes, goal_positions, cursor, num_boxes);
         
      
      insert_sorted_queue(states, new_state, compare_state);
   }
   
   // unset boxes to graph
   for (u_int i = 0 ; i < num_boxes; i++) 
      puzzle_temp[current_state->boxes[i].x][current_state->boxes[i].y] = 0;
   
   return 0;   
}

State *search_solution(char **puzzle, char **puzzle_temp, Queue *states,Queue *history, Coordinate *goal_positions, u_int num_boxes) {
   int nodes = 1;
   
   while (states->length > 0) {
      State *state = remove_head_queue(states);
      
      if (state->heuristic_score == 0.0) {
         printf("Found after %d nodes\n", nodes);
#ifdef DEBUG
         printf("history queue size: %d\n", history->length);
#endif
         return state;
      }
      make_move(puzzle, puzzle_temp, states, history, state, goal_positions, num_boxes);
      
      insert_head_queue(history, state);
      nodes++;
   }

#ifdef DEBUG
   printf("Nodes processed: %d\n", nodes);
   printf("history queue size: %d\n", history->length);
#endif
   
   return NULL;
}

int main(int argc, char **argv) {   

   char temp[20];
      
   if (fgets(temp, 20, stdin) == NULL)
      err_exit("Incorrect format of file");
      
   u_int puzzle_size = strtol(temp, NULL, 10);
   
   char line[PUZZLE_WIDTH_LIMIT];
   char **puzzle= malloc(sizeof(char *) * puzzle_size);
   char **puzzle_temp= malloc(sizeof(char *) * puzzle_size);
   
   if (puzzle== NULL)
      err_exit("Memory Error");
   
   Coordinate current_pos;
   u_int line_number = 0;
   u_int width = 0;
   u_int boxes_id = 0;
   u_int goal_pos_id = 0;
   Coordinate *boxes = malloc(sizeof(Coordinate)*puzzle_size*puzzle_size);
   Coordinate *goal_positions = malloc(sizeof(Coordinate)*puzzle_size*puzzle_size);
   
   while (fgets(line, PUZZLE_WIDTH_LIMIT, stdin) != NULL) {
#ifdef DEBUG
      printf("DEBUG->%s", line);
#endif
      if (line_number >= puzzle_size) {
         err_exit("Found too many lines while reading puzzle\n");
      }
      width = strlen(line);
      puzzle[line_number] = malloc(sizeof(char)*width);
      puzzle_temp[line_number] = malloc(sizeof(char)*width);
      if (puzzle[line_number] == NULL)
         err_exit("Memory Error");
      
      u_int i = 0;
      for (char *ptr = line; *ptr; ptr++) {
         puzzle_temp[line_number][i] = 0;
         
         switch (*ptr) {
            case ' ':
               puzzle[line_number][i] = ' ';
               break;
            case '#':
               puzzle[line_number][i] = '#';
               break;
            case '*':
               puzzle[line_number][i] = '.';
               goal_positions[goal_pos_id].x = line_number;
               goal_positions[goal_pos_id].y = i;
               goal_pos_id++;
            case '$':
               if (puzzle[line_number][i] != '.')
                  puzzle[line_number][i] = ' ';
               boxes[boxes_id].x = line_number;
               boxes[boxes_id].y = i;
               boxes_id++;
               break;
            case '+':
               current_pos.x = line_number;
               current_pos.y = i;
            case '.':
               puzzle[line_number][i] = '.';
               goal_positions[goal_pos_id].x = line_number;
               goal_positions[goal_pos_id].y = i;
               goal_pos_id++;
               break;              
            case '@':
               puzzle[line_number][i] = ' ';
               current_pos.x = line_number;
               current_pos.y = i;
               break;
            case '\n':
               break;
            default:
               free(puzzle);
               free(puzzle_temp);
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
   
   boxes = realloc(boxes, boxes_id*sizeof(Coordinate));
   goal_positions = realloc(goal_positions, goal_pos_id*sizeof(Coordinate));
   
   State *root_state = malloc(sizeof(State));
   root_state->parent = NULL;
   root_state->move_from_parent = 0;
   root_state->boxes = boxes;
   root_state->current_pos = &current_pos;
   root_state->cost_score = 0;
   root_state->heuristic_score = calculate_heuristic_score(root_state->boxes, goal_positions, &current_pos, boxes_id);

#ifdef DEBUG
   print_state(puzzle, root_state, goal_positions, boxes_id, width, line_number);
#endif
   
   Queue *states, *history;
   init_queue(&states);
   init_queue(&history);
   insert_head_queue(states, root_state);
   
   State *solution;
   
   if (NULL != (solution = search_solution(puzzle, puzzle_temp, states, history, goal_positions, boxes_id))) {
#ifdef DEBUG
      print_state(puzzle, solution,goal_positions, boxes_id, width, line_number);
#endif
      getSolution(solution);
      return 0;
   } else
      return 1;
}
