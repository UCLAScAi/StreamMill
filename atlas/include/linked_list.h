#define ACTIVE_SIZE 10


struct item_el {
   int val;
   struct item_el* next;
};

typedef struct hashC { 
  struct item_el* curr;
  struct item_el* head;
}hashC;

typedef struct item_el item;


hashC* activeGrids;
void insert(int c);
int deleteC(int c);
void print(int c);
void print_all();
int search(int c);

