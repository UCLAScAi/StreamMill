#include "atlaswin.h"
void displayData(char *data)
{
  printf("%d\n", *(int*)data);
}
int main()
{
  _adl_win_ctrl_t wc = _adl_newWinCtrl(_ADL_WIN_ROW, 1, 4/*key size*/, 4/* data size*/);
  _adl_win_t wins[10];
  int i;
  int key = 0;

  for (i=0; i<2000; i++) {
    //    int key = i % 10;
    wins[key]=_adl_winPutTuple(wc, (char*)&key, (char*)&i);
    _adl_winDisplay(wins[key], displayData);
  }

//   for (i=0; i<10;i++) {
//     printf("********WIN %d**********\n", i);
//     _adl_winDisplay(wins[i], displayData);
//   }
  _adl_deleteWinCtrl(wc);
}
