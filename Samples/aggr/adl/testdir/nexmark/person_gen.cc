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

static int NUM_FIRSTNAMES = 50;

static char* firstnames[] = {
	"Perla","Girolamo","Ronnie","Xianlong",
	"Xiping","Adas","Patricia","Renee","Masami","Snehasis","Gil",
	"Izaskun","Indrajit","Gio","Arun","Arvin","Carrsten","Arup",
	"Patricio","Rimon","Giordano","Xumin","Sumali","Roselyn","Annemarie",
	"Jiafu","Chinhyun","Eldridge","Stamatina","Stabislas","Jovan",
	"Christoph","Yonghong","Behnaam","Arve","Bernice","Georg","Marek",
	"Maren","Mountaz","Kiyomitsu","Irena","Munenori","Irene","Cullen",
	"Apostol","Mihalis","Xiong","Kazuhira","Itzchak"
 };

static int NUM_LASTNAMES = 50;
static char* lastnames[] = {
	"Wossner","Gunderson","Comte","Linnainmaa","Harbusch","Speek",
	"Trachtenberg","Kohling","Speel","Nollmann","Jervis","Capobianchi",
	"Murillo","Speer","Claffy","Lalonde","Nitta","Servieres","Chimia",
	"Boreale","Taubenfeld","Nitto","Walston","Danley","Billawala",
	"Ratzlaff","Penttonen","Pashtan","Iivonen","Setlzner","Reutenauer",
	"Hegner","Demir","Ramaiah","Covnot","Nitsch","Thummel","Axelband",
	"Sevcikova","Shobatake","Greibach","Fujisaki","Bugrara","Dolinsky",
	"Dichev","Versino","Gluchowski","Dahlbom","Suri","Parveen"
};

static int NUM_CITIES = 25;
static char* cities[] = {
    "Abidjan","Abu","Acapulco","Aguascalientes","Akron","Albany",
    "Albuquerque","Alexandria","Allentown","Amarillo","Amsterdam",
    "Anchorage","Appleton","Aruba","Asheville","Athens","Atlanta", // 
    "Augusta","Austin","Baltimore","Bamako","Bangor","Barbados",
    "Barcelona","Basel"
    };

static int NUM_PROVINCES = 51;
static char* provinces[] = {
  "AL","AK","AZ","AR","CA","CO",
  "CT","DE","DC","FL","GA",
  "HI","ID","IL","IN","IA","KS","KY",
  "LA","ME","MD","MA","MI","MN",
  "MS","MO","MT","NE","NV","NH",
  "NJ","NM","NY","NC","ND",
  "OH","OK","OR","PA","RI",
  "SC","SD","TN","TX","UT","VT",
  "VA","WA","WV","WI","WY"
}
;


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

  int totalNum = 500;

  ofstream out("persons.txt");
  
  for (int i = 1; i <= totalNum; i++) {
    int id = i;
    char* firstName = firstnames[rand()%NUM_FIRSTNAMES];
    char* lastName = lastnames[rand()%NUM_LASTNAMES];    
    char name[21], email[21], city[21], state[3];
    strcpy(name, firstName);
    strcat(name, " ");
    strcat(name, lastName);
    strcpy(email, lastName);
    strcat(email, "@usermail.com");
    int creditCard = rand()% 1000000000000000;
    strcpy(city, cities[rand()%NUM_CITIES]);
    strcpy(state, provinces[rand()%NUM_PROVINCES]); //state

    //write to file
    if(!out){
      cout <<"Error, cannot open file for output" << endl;
      return 1;
    }
    out << id << "," << name << "," << email << "," << creditCard << "," << city << "," << state << endl;
  }
  
  out.flush();
  out.close();
  return 0;
}
