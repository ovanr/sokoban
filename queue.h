#ifndef QUEUE_H

typedef unsigned char u_char;
typedef unsigned int u_int;

typedef struct node {
   void *data;
   struct node *next;
} Node;

typedef struct {
   Node *head;
   Node *tail;
   u_int length;
} Queue;

int insert_head_queue(Queue *ptr, void *data);

int insert_tail_queue(Queue *ptr, void *data);

// compare function should return 1 if an item in queue is > than the new item
//                                0 if they are equal
//                                -1 if the item in queue is < than the new item
int insert_sorted_queue(Queue *ptr, void *data, int (*compare)(void *, void *));

void *remove_head_queue(Queue *ptr);

void *remove_tail_queue(Queue *ptr);

void free_full_queue(Queue *ptr);

int init_queue(Queue **ptr);

#endif
