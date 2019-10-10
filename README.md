# Automated Sokoban Puzzle solver

**_This program assumes the puzzle given is surrounded by walls <br/>so no checks are made to see if a box or the cursor moves out of boundaries_**

## MakeFile
```
    make clean  -- to clear compiled files
    make        -- to create normal executable
    make debug  -- to create extra verbose executable
```
    
## Sokoban
`usage: ./sokoban [--help] | [--silent] [heuristic algorithm]`

Simple Sokoban puzzle solver<br/>
Puzzle is read from stdin<br/>

Default heuristic algorithm is match_closest<br/>
   
Available Heuristic Algorithms:<br/>
- count_boxes      *Count the number of boxes that aren't in goal position only*
- fixed_penalty    *find the minimum distance from an unmatched box to a goal position, and add that plus the (number of unmatched boxes-1) to final score*
- coarse_match     *NON OPTIMAL, see source file*
- match_closest    *for each box get its minimum distance from a goal position and sum it all up (multiple boxes can be matched on the same goal position)*
<br/>
All the algorithms except the count_boxes, also add the minimum distance of the cursor from an unmatched box optional arguments:
- --help                  show this help message and exit
- --silent                Don't print intermediary states
   
*(where distance is defined as the difference of steps in the x direction and in the y direction, assuming no obstacles in between)*
