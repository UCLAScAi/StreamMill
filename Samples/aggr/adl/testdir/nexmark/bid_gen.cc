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

static int prices[2500]; //store auction initial prices
static int bidPrices[2500]; //store auction bid prices generated so far

int main(int argc, char**argv){

  //initialize price arrays
  ifstream in("prices.txt");

  if(!in) {
    cout << "no such file" << endl;
    return 1;
  }
  
  char read[6];
  int count = 0;
  while(!in.eof()) {
    //while eof hasnt occured
    in.getline(read, 6);
    prices[count] = atoi(read);
    bidPrices[count] = atoi(read);
    count++;    
  }
  
  srand( time(NULL) );
  
  int totalNum = 10000;

  ofstream out("bids.txt");
  
  //suppose bids come in at speed up to 4 times faster than auctions
  //it is up to 20 times faster than user entering speed.
  for (int i = 1; i <= totalNum; i++) {
    int aucId;    
    int maxAuction = i/4;
    if (maxAuction == 0) aucId = 1;
    else {
      aucId = (rand()%i/8)%2500 + i/8 + 1;
    }

    //make the bid price anywhere between 1 to 5 higher from last price
    int price = (rand()%5 + 1) + bidPrices[aucId];
    bidPrices[aucId] = price;

    int bidder;
    int maxBidder = i/20;
    if (maxBidder == 0) bidder = 1;
    else {
      bidder = (rand()%maxBidder)%500 + 1;
    }
    
    //write to file
    if(!out){
      cout <<"Error, cannot open file for output" << endl;
      return 1;
    }
    out << aucId << "," << price << "," << bidder << endl;
  }
  
  out.flush();
  out.close();
  
  return 0;
}
