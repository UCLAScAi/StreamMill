#include<stdlib.h>
#include<stdio.h>
#include "./linked_list.h"


void insert(int c) {
   activeGrids[c%ACTIVE_SIZE].curr = (item*)malloc(sizeof(item));
   activeGrids[c%ACTIVE_SIZE].curr->val = c;
   activeGrids[c%ACTIVE_SIZE].curr->next = activeGrids[c%ACTIVE_SIZE].head; 
   activeGrids[c%ACTIVE_SIZE].head = activeGrids[c%ACTIVE_SIZE].curr;
}

int delete(int c) {
   if (activeGrids[c%ACTIVE_SIZE].head->val == c) { 
      activeGrids[c%ACTIVE_SIZE].head = activeGrids[c%ACTIVE_SIZE].head->next;
      return 1;  
   }
activeGrids[c%ACTIVE_SIZE].curr = activeGrids[c%ACTIVE_SIZE].head;
while (activeGrids[c%ACTIVE_SIZE].curr->next){ 
   if (activeGrids[c%ACTIVE_SIZE].curr->next->val == c) { 
      activeGrids[c%ACTIVE_SIZE].curr->next = activeGrids[c%ACTIVE_SIZE].curr->next->next;
      return 1;  
   }
 //  prev = cur; 
  // cur = cur->next;
   activeGrids[c%ACTIVE_SIZE].curr = activeGrids[c%ACTIVE_SIZE].curr->next;
}
}

int search(int c) {
item* head = activeGrids[c%ACTIVE_SIZE].head;
while (head){ 
   if (head->val == c){ 
      return 1; 
}   head = head->next;
}
return 0;
}
void print(int index) {
item* head = activeGrids[index].head;

while (head){ 
   printf("%i ", head->val);
   head = head->next;
}
printf("\n");

}

void print_all() {
int i;

for (i=0; i<ACTIVE_SIZE; i++) {

item* head = activeGrids[i].head;

while (head){ 
   printf("%i ", head->val);
   head = head->next;
}

printf("\n");
}
}

/*
int main() {
 int i;
 activeGrids = (hash*)malloc(ACTIVE_SIZE*sizeof(hash));
 printf("inserting \n");
 insert(1);
 insert(11);
 insert(21);
 insert(41);
 printf("after inserting \n");
 print(1);
 printf("searching.. \n");
 if (search(21))
   printf("found 21! \n");
 printf("deleting.. 21\n");
 delete(21);
 print(1);
 insert(341);
 insert(51);
 insert(91);
 insert(191);
 print(1);
 
*/
/*
   item * curr, * head;
   int i;

   head = NULL;

   for(i=1;i<=ACTIVE_SIZE;i++) {
      curr = (item *)malloc(sizeof(item));
      curr->val = i;
      curr->next  = head;
      head = curr;
   }

   curr = head;

   while(curr) {
      printf("%d\n", curr->val);
      curr = curr->next ;
   }
*/
