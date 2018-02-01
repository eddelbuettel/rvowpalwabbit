#include <unistd.h>		// close()
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>

#include <Rcpp.h>
#define VWCOUT Rcpp::Rcout

using std::cin;
using std::endl;
//using std::cout;
using std::cerr;
using std::string;


int open_socket(const char* host, unsigned short port)
{
  hostent* he;
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
  return sd;
}

int recvall(int s, char* buf, int n){
    int total=0;
    int ret=recv(s, buf, n, 0);
    while(ret>0 && total<n){
        total+=ret;
        if(buf[total-1]=='\n')
            break;
        ret=recv(s, buf+total, n, 0);
    }
    return total;
}

int main(int argc, char* argv[]){
    char buf[256]; 
    char* toks;
    const char* host="localhost";
    unsigned short port=~0;
    ssize_t pos;
    int s,ret,queries=0;
    string line;
    
    if(argc>1){
        host = argv[1];
    }
    if(argc>2){
        port=atoi(argv[2]);
    }
    if(port <= 1024 || port==(unsigned short)(~0)){
        port = 26542;
    }
    
    s=open_socket(host, port);
    size_t id=0;
    ret=send(s,&id,sizeof(id),0);
    if(ret<0){
        Rf_error("Could not perform handshake!");
    }
    
    while(getline(cin,line)){
        line.append("\n");
        int len=line.size();
        const char* cstr = line.c_str();
        const char* sp = strchr(cstr,' ');
        ret=send(s,sp,len-(sp-cstr),0);
        if(ret<0){
	  Rf_error("Could not send unlabeled data!");
        }
        ret=recvall(s, buf, 256);
        if(ret<0){
	  Rf_error("Could not receive queries!");
        }
        buf[ret]='\0';
        toks=&buf[0];
        strsep(&toks," ");      // just advance toks
        strsep(&toks," ");      // and again
        char* itok=strsep(&toks,"\n");
        if(itok==NULL || itok[0]=='\0'){
            continue;
        }

        queries+=1;
        string imp=string(itok)+" |";
        pos = line.find_first_of ("|");
        line.replace(pos,1,imp); 
        cstr = line.c_str();
        len = line.size();
        ret = send(s,cstr,len,0);
        if(ret<0){
	  Rf_error("Could not send labeled data!");
        }
        ret=recvall(s, buf, 256);
        if(ret<0){
	  Rf_error("Could not receive predictions!");
        }
    }
    close(s);
    VWCOUT << "Went through the data by doing " << queries << " queries" << endl;
    return 0;
}

