#ifndef __GUICLIENT_H__
#define __GUICLIENT_H__

class GUIClient {
  char* clientIp;
  int port;

  public:
    GUIClient(char* clientIp, int port);
    int getPort();
    char* getClientIp();
};

#endif
