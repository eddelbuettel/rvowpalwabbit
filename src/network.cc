#include <unistd.h>	// write()
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <netdb.h>
#include <strings.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <iostream>

#include <Rcpp.h>
#define VWCOUT Rcpp::Rcout

using namespace std;

int open_socket(const char* host)
{
  const char* colon = index(host,':');
  short unsigned int port = 26542;
  hostent* he;
  if (colon != NULL)
    {
      port = atoi(colon+1);
      string hostname(host,colon-host);
      he = gethostbyname(hostname.c_str());
    }
  else
    he = gethostbyname(host);

  if (he == NULL)
    {
      Rf_error("can't resolve hostname: %s", host);
    }
  int sd = socket(PF_INET, SOCK_STREAM, 0);
  if (sd == -1)
    {
      Rf_error("can't get socket ");
    }
  sockaddr_in far_end;
  far_end.sin_family = AF_INET;
  far_end.sin_port = htons(port);
  far_end.sin_addr = *(in_addr*)(he->h_addr);
  memset(&far_end.sin_zero, '\0',8);
  if (connect(sd,(sockaddr*)&far_end, sizeof(far_end)) == -1)
    {
      Rf_error("can't connect to: %s:%d", host, port);
    }
  char id = '\0';
  if (write(sd, &id, sizeof(id)) < (int)sizeof(id))
    VWCOUT << "write failed!" << endl;
  return sd;
}
