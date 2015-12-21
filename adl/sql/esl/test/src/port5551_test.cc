#include "portx.h"

int main() {

  unordered_map <string, string> bucket;
  char buf[1000];
  sprintf(buf, "%s", "smm_traffic,input_1,input_2,input_3\nsmm_stocks,input_1,input_2\nsmm_traffic,input_1, input_2,input_3");
  BucketizeStream(bucket, &buf[0]);
  printf("After execution contents of the hash map are: %s and stock %s\n", bucket["smm_traffic"].c_str(), bucket["smm_stocks"].c_str());
  return 1;
}
