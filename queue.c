#include <stdlib.h>
#include <string.h>
#include "queue.h"

int insert_head_queue(Queue *ptr, void *data, u_int data_size)  {
   Node *new_node = malloc(sizeof(Node));
   new_node->data = malloc(sizeof(char)*data_size);
   
   if (new_node == NULL)
      return 1;
   
   if (new_node->data == NULL) {
      free(new_node);
      return 1;
   }
   if (NULL == memcpy(new_node->data, data, data_size)) {
      free(new_node->data);
      free(new_node);
      return 1;
   }
   new_node->next = ptr->head;
   ptr->head = new_node;
   ptr->length++;
   if (ptr->length == 1)
      ptr->tail = new_node;
   
   return 0;
}

int insert_tail_queue(Queue *ptr, void *data, u_int data_size)  {
   Node *new_node = malloc(sizeof(Node));
   new_node->data = malloc(sizeof(char)*data_size);
   
   if (new_node == NULL)
      return 1;
   
   if (new_node->data == NULL) {
      free(new_node);
      return 1;
   }
   if (NULL == memcpy(new_node->data, data, data_size)) {
      free(new_node->data);
      free(new_node);
      return 1;
   }
   new_node->next = NULL;
   ptr->length++;
   
   if (ptr->length == 1)
      ptr->tail = ptr->head = new_node;
   else {
      ptr->tail->next = new_node;
      ptr->tail = new_node;
   }
   
   return 0;
}

int insert_sorted_queue(Queue *ptr, void *data, u_int data_size, int (*compare)(void *, void *))  {
   Node *new_node = malloc(sizeof(Node));
   new_node->data = malloc(sizeof(char)*data_size);
   
   if (new_node == NULL)
      return 1;
   if (new_node->data == NULL) {
      free(new_node);
      return 1;
   }
   if (NULL == memcpy(new_node->data, data, data_size)) {
      free(new_node->data);
      free(new_node);
      return 1;
   }
   
   Node *counter = ptr->head;
   Node *previous = NULL;
   while (counter != NULL) {
      if (compare(counter->data, data) >= 0) 
         break;
      previous = counter;
      counter = counter->next;
   }
   
   new_node->next = counter;
   ptr->length++;
   
   if (previous == NULL)
      ptr->head = new_node;
   else
      previous->next = new_node;
   
   if (counter == NULL)
      ptr->tail = new_node;

   
   return 0;
}

int remove_head_queue(Queue *ptr) {
   if (ptr->head == NULL)
      return 1;
   
   free(ptr->head->data);
   ptr->length--;
   if (ptr->length == 0) {
      free(ptr->head);
      ptr->head = ptr->tail = NULL;
   } else {
      Node *temp = ptr->head;
      ptr->head = ptr->head->next;
      free(temp);
   }
   return 0;
}

int remove_tail_queue(Queue *ptr) {
   if (ptr->tail == NULL)
      return 1;
   
   free(ptr->tail->data);
   ptr->length--;
   if (ptr->length == 0) {
      free(ptr->tail);
      ptr->head = ptr->tail = NULL;
   } else {
      Node *counter = ptr->head;
      while (counter->next != ptr->tail)
         counter = counter->next;
      
      free(ptr->tail);
      ptr->tail = counter;
      ptr->tail->next = NULL;
   }
   return 0;
}

void free_full_queue(Queue *ptr) {
   Node *counter = ptr->head;
   while (counter != NULL) {
      free(counter->data);
      Node *temp = counter;
      counter = counter->next;
      free(temp);
   }
   free(ptr);
}      
      
int init_queue(Queue **ptr) {
   *ptr = malloc(sizeof(Queue));
   if (*ptr == NULL) 
      return 1;
   
   (*ptr)->length = 0;
   return 0;
}

#ifdef QDEBUG

#include <stdio.h>
struct person {
   int a;
   int b;
   int c;
};

void print_queue(Queue *ptr) {
   Node *counter = ptr->head;
   if (counter == NULL)
      return;
   struct person *h = ptr->head->data;
   struct person *t = ptr->tail->data;
   
   printf("head= %d, tail=%d\n", h->a, t->a); 
   while (counter != NULL) {
      struct person *p = counter->data;
      printf("a= %d, b= %d, c=%d\n", p->a, p->b, p->c);
      counter = counter->next;
   }
}

int compare(void *a, void *b) {
   struct person *aa = a;
   struct person *bb = b;
   
   if (aa->a > bb->a)
      return 1;
   
   if (aa->a == bb->a)
      return 0;
   
   return -1;
}

int main(void) {
   Queue *ptr;
   printf("%d\n", init_queue(&ptr));

   struct person mydata;
   mydata.a = 3;
   mydata.b = 2;
   mydata.c = 3;
   
   printf("%d\n", insert_sorted_queue(ptr, &mydata, sizeof(struct person), compare));
   mydata.a = 1;
   print_queue(ptr);
   printf("%d\n", insert_sorted_queue(ptr, &mydata, sizeof(struct person), compare));
   mydata.a = 2;         
   print_queue(ptr);
   printf("%d\n", insert_sorted_queue(ptr, &mydata, sizeof(struct person), compare));
   mydata.a = 5;        
   print_queue(ptr);
   printf("%d\n", insert_sorted_queue(ptr, &mydata, sizeof(struct person), compare));
   mydata.a = 4;       
   print_queue(ptr);
   printf("%d\n", insert_sorted_queue(ptr, &mydata, sizeof(struct person), compare));
   
   print_queue(ptr);
   printf("%d\n", remove_tail_queue(ptr));
   
   print_queue(ptr);
   printf("%d\n", remove_tail_queue(ptr));
   print_queue(ptr);
   printf("%d\n", remove_tail_queue(ptr));
   print_queue(ptr);
   printf("%d\n", remove_tail_queue(ptr));
   print_queue(ptr);
   printf("%d\n", remove_tail_queue(ptr));
   print_queue(ptr);
   printf("%d\n", remove_tail_queue(ptr));
   print_queue(ptr);
   printf("%d\n", remove_tail_queue(ptr));
   print_queue(ptr);
   printf("%d\n", remove_tail_queue(ptr));
   free_full_queue(ptr);
   return 0;
}

#endif
