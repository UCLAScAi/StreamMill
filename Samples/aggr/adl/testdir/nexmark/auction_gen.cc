#include <sys/types.h>
#include <sys/param.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <iostream.h>
#include <fstream.h>

using namespace std;

#define TOTAL_NUM_ITEMS 500
#define TOTAL_NUM_CATEGORIES 25

static int last_item_id = 0;
static char* items[TOTAL_NUM_ITEMS]; //store generated items
static int categories[TOTAL_NUM_ITEMS]; //store categories for corresponding items
static int prices[2500]; //store generated prices

void stringPad(char* src, char* dest, int length)
{
  int strlength = strlen(src);
  int i = 0;
  for(i; i < strlength; i++)
  {
    dest[i] = src[i];
  }
  for(i; i < length-1; i++)
  {
    dest[i] = ' ';
  }
  dest[length-1] = '\0';
}

int main(int argc, char**argv){

  srand( time(NULL) );

  int itemCount = 0;
  int totalNum = 2500;

  ofstream out("auctions.txt");
  
  //suppose auctions come in at speed up to 5 times faster
  for (int i = 1; i <= totalNum; i++) {
    int id = i;
    
    char itemName[11];
    int catId;
    int price;
    int itemId = rand()%TOTAL_NUM_ITEMS+1;
    
    if (itemId > last_item_id) {
      itemId = ++last_item_id;
      sprintf(itemName, "Item %d", itemId);
      catId = rand()%TOTAL_NUM_CATEGORIES+1;
      items[itemId-1] = strdup(itemName);
      categories[itemId-1] = catId;
      price = rand()%(itemId*5);
      itemCount++;
    } else {
      strcpy(itemName, items[itemId-1]);
      catId = categories[itemId-1];
      price = prices[itemId-1];
    }

    prices[i-1] = price;
      
    int sellerId;     
    int maxSeller = i/5;
    if (maxSeller == 0) sellerId = 1;
    else {
      sellerId = (rand()%maxSeller)%500 + 1;
    }

    //make the expiration date anywhere between 5 sec to 2 min from now
    //in the file, only store this difference, calculate on the fly when reading in
    int exp_diff = (rand()%120 + 5);
    
    //write to file
    if(!out){
      cout <<"Error, cannot open file for output" << endl;
      return 1;
    }
    out << id << "," << itemName << "," << sellerId << "," << price << "," << catId << "," << exp_diff << endl;
  }
  
  out.flush();
  out.close();

  ofstream out2("prices.txt");
  for (int i = 0; i < 2500; i++) {
    out2 << prices[i] << endl;
  }
  
  return 0;
}
