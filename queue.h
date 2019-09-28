#ifndef QUEUE_H

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

// data_size = number of bytes of the content pointed to by the data pointer
int insert_head_queue(Queue *ptr, void *data, u_int data_size);

int insert_tail_queue(Queue *ptr, void *data, u_int data_size);

// compare function should return 1 if an item in queue is > than the new item
//                                0 if they are equal
//                                -1 if the item in queue is < than the new item
int insert_sorted_queue(Queue *ptr, void *data, u_int data_size, int (*compare)(void *, void *));

int remove_head_queue(Queue *ptr);

int remove_tail_queue(Queue *ptr);

void free_full_queue(Queue *ptr);

int init_queue(Queue **ptr);

#endif
