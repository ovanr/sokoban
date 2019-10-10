/*
 sokoban.c Copyright (C) 2019 Orpheas van Rooij
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include "queue.h"
#include <math.h>
#include <string.h>
#include <limits.h>

#define True 1
#define False 0

#define PUZZLE_WIDTH_LIMIT 200
#define OPTIMALITY_STRICTNESS 1  // 1 is for optimal, higher values sacrifice optimality for speed and memory

typedef struct {
   int x;
   int y;
} Coordinate;

// Lightweight state, only boxes coordinates and cursor coordinates are stored
typedef struct state {
   u_char move_from_parent;
   struct state *parent;
   Coordinate *current_pos;
   Coordinate *boxes;
   int cost_score;
   u_int heuristic_score;
} State;
   
void err_exit(char *msg) {
   fprintf(stderr, "%s\n", msg);
   exit(1);
}

// (A) find the box with the lowest distance from goal position
// (B) and for the rest boxes that aren't in goal position add a penalty per box given by OPTIMALITY_STRICTNESS.
// for the heuristic to be fully optimal, the OPTIMALITY_STRICTNESS assumes you will only need one move
// per box to move to goal position. you can change this to something more reasonable but optimality may be sacrificed.
// the distance of the current cursor compared to the box in (A) is added to the final score as well.
u_int heuristic_fixed_penalty(Coordinate *boxes, Coordinate *goal_positions, Coordinate *cursor, u_int num_boxes) {
   u_int total_score = 0;
   u_int mismatched_boxes_count = num_boxes;
   u_int cursor_closest = 0;
   
   u_int global_min_score = UINT_MAX;
   
   for (u_int i = 0; i < num_boxes; i++) {
      u_int local_min_score = UINT_MAX;
      u_int score;
      for (u_int c = 0; c < num_boxes; c++) {
         
         // get score for box i and pos i
         score = abs(boxes[i].x-goal_positions[c].x) + \
         abs(boxes[i].y-goal_positions[c].y);
         if (score == 0)
            break;
         
         if (local_min_score > score) 
            local_min_score = score;
         
      }
      if (score == 0) // if the box isn't in goal position then we can count its minimum score
         mismatched_boxes_count--;
      else if (global_min_score > local_min_score) {
         cursor_closest =  abs(boxes[i].x-cursor->x) + \
                              abs(boxes[i].y-cursor->y);
         global_min_score = local_min_score;
      }
   }
   total_score = (mismatched_boxes_count == 0) ? 0 : (mismatched_boxes_count*OPTIMALITY_STRICTNESS)+global_min_score;
   return total_score + cursor_closest;
}

// calculate distance between boxes and goal positions in a fifo priority
// manner, so the first box in boxes array chooses from whole array, then the second
// box chooses from the remaining N-1, and so on. 
// NON-OPTIMAL
// the minimum distance of the current cursor compared to all the boxes is added to the final score as well.
u_int heuristic_coarse_match(Coordinate *boxes, Coordinate *goal_positions, Coordinate *cursor, u_int num_boxes) {
   u_int total_score = 0;
   _Bool used_goal_positions[num_boxes];
   u_int cursor_closest = UINT_MAX;
   
   for (u_int x =0; x < num_boxes; x++)
      used_goal_positions[x] = False;
   
   for (u_int i = 0; i < num_boxes; i++) {

      
      u_int local_min_score = UINT_MAX;
      u_int min_box = 0;
      for (u_int c = 0; c < num_boxes; c++) {
         if (used_goal_positions[c] == True)
            continue;
         
         // get score for box i and pos i
         u_int score = abs(boxes[i].x-goal_positions[c].x) + \
                           abs(boxes[i].y-goal_positions[c].y);
         
         
         if (local_min_score > score) {
            local_min_score = score;
            min_box = c;
         }
      }
      if (local_min_score != 0) {
         u_int temp =  abs(boxes[i].x-cursor->x) + \
                  abs(boxes[i].y-cursor->y);
         if (temp < cursor_closest)
            cursor_closest = temp;
      }
      used_goal_positions[min_box] = True;
      total_score += local_min_score;
   }
   
   return (total_score == 0) ? 0 : total_score + cursor_closest;

}

// calculate the distance between the boxes and all the goal positions 
// and add the minimum distance per box to final score.
// OPTIMAL
// the minimum distance of the current cursor compared to all the boxes is added to the final score as well.
u_int heuristic_match_closest(Coordinate *boxes, Coordinate *goal_positions, Coordinate *cursor, u_int num_boxes) {
   u_int total_score = 0;   
   u_int cursor_closest = UINT_MAX;
   
   for (u_int i = 0; i < num_boxes; i++) {

      
      u_int local_min_score = UINT_MAX;
      for (u_int c = 0; c < num_boxes; c++) {
         
         // get score for box i and pos i
         u_int score = abs(boxes[i].x-goal_positions[c].x) + \
                        abs(boxes[i].y-goal_positions[c].y);
         
         if (score < local_min_score) 
            local_min_score = score;
         
      }
      if (local_min_score != 0) {
         u_int temp = abs(boxes[i].x-cursor->x) + \
                              abs(boxes[i].y-cursor->y);
         if (temp < cursor_closest)
            cursor_closest = temp;
      }
      total_score += local_min_score;
   }
   if (total_score == 0)
      return 0;
   total_score += cursor_closest;
   return total_score;
}

// calculate the number of boxes not in goal positions 
// OPTIMAL
u_int heuristic_count_boxes(Coordinate *boxes, Coordinate *goal_positions, Coordinate *cursor, u_int num_boxes) {
   u_int total_score = num_boxes;   
   
   for (u_int i = 0; i < num_boxes; i++) {
      for (u_int c = 0; c < num_boxes; c++) 
         // check if it is in goal position
         if ((goal_positions[c].x == boxes[i].x) && \
            (goal_positions[c].y == boxes[i].y)) {
            total_score--;
         }
   }

   return total_score;
}

int compare_state(void *present_state, void *new_state) {
   State *ps = present_state;
   State *ns = new_state;
   u_int p_score = ps->heuristic_score + ps->cost_score;
   u_int n_score = ns->heuristic_score + ns->cost_score;
   
   if (p_score > n_score)
      return 1;
   if (p_score < n_score)
      return -1;
   
   return 0;
}

/* deadlock scenarios:
*         if there is a wall on at least one side of the box in the x-axis AND in the y-axis
*         if a box is locked and can only move to one direction only and in that direction there are no goal positions 
* !!! a box is counted as a space, meaning only walls are taken into consideration, so deadlocks caused by boxes sticked together goes undetected!!!
*/
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
      
      // check if box can move to a single direction only
      // and further check if there is a goal in that direction it can move
      // if there isn't then DEADLOCK
      
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

void print_state(char **puzzle, State *sol, int num_boxes, int puzzle_length) {
   printf("-----------------\n");
   
   char **puzzle_cpy = malloc(sizeof(char *) * puzzle_length);
   
   for (int i = 0; i < puzzle_length; i++) {
      puzzle_cpy[i] = malloc(sizeof(char) * (strlen(puzzle[i])+1));
      strcpy(puzzle_cpy[i], puzzle[i]);
   }

   for (int i = 0; i < num_boxes; i++) 
       puzzle_cpy[sol->boxes[i].x][sol->boxes[i].y] = ( puzzle_cpy[sol->boxes[i].x][sol->boxes[i].y] == '.') ? '*' : '$';
   
   puzzle_cpy[sol->current_pos->x][sol->current_pos->y] = ( puzzle_cpy[sol->current_pos->x][sol->current_pos->y] == '.') ? '+' : '@';
   
   
   printf("move score:    %d\n", sol->cost_score);
   printf("heuristic:    %d\n\n", sol->heuristic_score);
   
   for (int i = 0; i < puzzle_length; i++) 
      printf("%s\n", puzzle_cpy[i]);
   
   printf("-----------------\n");
      
}

State *get_duplicate(Queue *queue, Coordinate *new_cursor_pos, Coordinate *new_boxes, int num_boxes) {

   Node *ptr = queue->head;
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

void getSolution(State *sol, char *str) {
   if (sol->parent == NULL)
      return;
   getSolution(sol->parent, str);
   
   switch (sol->move_from_parent) {
      case (0): strcat(str, "up ");break;
      case (1): strcat(str, "down ");break;
      case (2): strcat(str, "left ");break;
      case (3): strcat(str, "right ");break;
   }
   
}   

/*
 * to save space, we use lightweight states, meaning only the coordinates of the boxes are stored in each state.
 * But because a puzzle matrix is useful to help in the computations, the boxes of the state are
 * inserted to the puzzle at each run of this function and then removed
 * 
 * So the puzzle variable holds the puzzle matrix with only the walls and the goal positions
 * and the puzzle_temp is an empty puzzle matrix that we use in this function to insert the boxes
 * coordinates in, and then upon finishing of the function we clear it
 * 
 * Given one root state this function will attempt to make all other four children from this state (all 4 positions)
 * but a child is not inserted if:
 *         Satisfied the deadlock criteria (see simple_deadlock_detect function)
 *         is found in the history or running queue. (if it is found but the new state has lower move score, the states are replaced)
 */
int make_move(char **puzzle, char **puzzle_temp, Queue *states, Queue *history, State *current_state, Coordinate *goal_positions, u_int num_boxes, u_int (*heuristic_func)(Coordinate *, Coordinate *, Coordinate *, u_int),u_int puzzle_size, _Bool verbose) {  
   
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
      new_state->heuristic_score = heuristic_func(new_boxes, goal_positions, cursor, num_boxes);
      
      if (verbose) {
         printf("Accepted Move: %d \n", mv);
         print_state(puzzle, new_state, num_boxes, puzzle_size);
      }
      
      insert_sorted_queue(states, new_state, compare_state);
   }
   
   // unset boxes to graph
   for (u_int i = 0 ; i < num_boxes; i++) 
      puzzle_temp[current_state->boxes[i].x][current_state->boxes[i].y] = 0;
   
   return 0;   
}

State *search_solution(char **puzzle, char **puzzle_temp, Queue *states,Queue *history, Coordinate *goal_positions, u_int num_boxes, u_int (*heuristic_func)(Coordinate *, Coordinate *, Coordinate *, u_int), u_int puzzle_size, _Bool verbose) {
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
      if (verbose) {
         printf("\n#########################\nExpanding State: \n");
         print_state(puzzle, state, num_boxes, puzzle_size);
      }
      make_move(puzzle, puzzle_temp, states, history, state, goal_positions, num_boxes, heuristic_func, puzzle_size, verbose);
      if (verbose) {
         printf("#########################\n");
      }
      
      insert_head_queue(history, state);
      nodes++;
   }

#ifdef DEBUG
   printf("Nodes processed: %d\n", nodes);
   printf("history queue size: %d\n", history->length);
#endif
   
   return NULL;
}

void help(char *prog_name) {
   printf("usage: %s [--help] | [--silent] [heuristic algorithm]\n\
\n\
   Simple Sokoban puzzle solver\n\
   Puzzle is read from stdin\n\
\n\
   Default heuristic algorithm is fixed_penalty\n\
   Available Heuristic Algorithms:\n\
      count_boxes\n\
      fixed_penalty\n\
      coarse_match\n\
      match_closest\n\
\n\
   optional arguments:\n\
   --help                  show this help message and exit\n\
   --silent                Don't print intermediary states\n", prog_name);
   exit(1);
}   
   
int main(int argc, char **argv) { 
   _Bool verbose = True;
   u_int ind = 1;
   
   if (argc >= 2) {
      if (strcmp(argv[1], "--silent") == 0) {
         ind++;
         verbose = False;
      } else if (strcmp(argv[1], "--help") == 0)
         help(argv[0]);
   }
   
   u_int (*heuristic_funct)(Coordinate *, Coordinate *, Coordinate *, u_int) = heuristic_fixed_penalty;
   if (argc > ind) {
      if (strcmp(argv[ind], "count_boxes") == 0)
         heuristic_funct = heuristic_count_boxes;
      else if (strcmp(argv[ind], "fixed_penalty") == 0)
         heuristic_funct = heuristic_fixed_penalty;
      else if (strcmp(argv[ind], "coarse_match") == 0)
         heuristic_funct = heuristic_coarse_match;
      else if (strcmp(argv[ind], "match_closest") == 0)
         heuristic_funct = heuristic_match_closest;
      else {
         char buff[100];
         snprintf(buff, 100, "Unrecognised Algorithm: %s\n", argv[ind]); 
         err_exit(buff);
      }
   }
   
   char temp[20];
      
   if (fgets(temp, 20, stdin) == NULL)
      err_exit("Incorrect format of file");
      
   u_int puzzle_size = strtol(temp, NULL, 10);
   
   char line[PUZZLE_WIDTH_LIMIT];
   char **puzzle= malloc(sizeof(char *) * puzzle_size);
   char **puzzle_temp= malloc(sizeof(char *) * puzzle_size);
   
   if (puzzle== NULL)
      err_exit("Memory Error");
   
   u_int line_number, width, boxes_id, goal_pos_id;
   line_number = width = boxes_id = goal_pos_id = 0;
   
   Coordinate current_pos;
   Coordinate *boxes = malloc(sizeof(Coordinate)*puzzle_size*puzzle_size);
   Coordinate *goal_positions = malloc(sizeof(Coordinate)*puzzle_size*puzzle_size);
   
   while (fgets(line, PUZZLE_WIDTH_LIMIT, stdin) != NULL) {

      if (line_number >= puzzle_size) {
         err_exit("Found too many lines while reading puzzle\n");
      }
      width = strlen(line);
      puzzle[line_number] = malloc(sizeof(char)*width);
      puzzle_temp[line_number] = malloc(sizeof(char)*width);
      
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
               
               boxes[boxes_id].x = line_number;
               boxes[boxes_id].y = i;
               boxes_id++;
               break;
            case '$':
               puzzle[line_number][i] = ' ';
               
               boxes[boxes_id].x = line_number;
               boxes[boxes_id].y = i;
               boxes_id++;
               break;
            case '+':
               puzzle[line_number][i] = '.';
               current_pos.x = line_number;
               current_pos.y = i;
               
               goal_positions[goal_pos_id].x = line_number;
               goal_positions[goal_pos_id].y = i;
               goal_pos_id++;
               break; 
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
      
      puzzle[line_number][i] = 0; 
      puzzle_temp[line_number][i] = 0; 
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
   root_state->heuristic_score = heuristic_funct(root_state->boxes, goal_positions, &current_pos, boxes_id);
   
   Queue *states, *history;
   init_queue(&states);
   init_queue(&history);
   insert_head_queue(states, root_state);
   
   State *solution;
   
   if (NULL != (solution = search_solution(puzzle, puzzle_temp, states, history, goal_positions, boxes_id, heuristic_funct, line_number, verbose))) {

      print_state(puzzle, solution, boxes_id, line_number);

      char *out_str = malloc(sizeof(char) * (solution->cost_score*6 + 1));
      out_str[0] = 0;
      getSolution(solution, out_str);
      out_str[strlen(out_str)-1] = 0; // remove space at end
      printf("%s\n", out_str);
      free(out_str);
      return 0;
   }
   
   return 1;
}
