#include "GUIClient.h"

/*
 * GUIClient
 */
GUIClient::GUIClient(char* clientIp1, int port1)
{
  clientIp = clientIp1;
  port = port1;
}

char* GUIClient::getClientIp()
{
  return clientIp;
}

int GUIClient::getPort()
{
  return port;
}

