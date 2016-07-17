/*
Copyright (c) 2011 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license

This implements the allreduce function of MPI.  Code primarily by
Alekh Agarwal and John Langford, with help Olivier Chapelle.

 */
#include <unistd.h>		// close()
#include <iostream>
#include <cstdio>
#include <cmath>
#include <ctime>
#include <sys/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <netdb.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <sys/timeb.h>
#include "allreduce.h"

#include <Rcpp.h>
#define VWCOUT Rcpp::Rcout

using namespace std;
string current_master="";

node_socks socks;

float add(float* arr, int n) {
  float sum = 0.0;
  for(int i = 0;i < n;i++)
    sum += arr[i];
  return sum;
}

void printfloat(float* arr, int n) {
  for(int i = 0;i < n;i++)
    VWCOUT<<arr[i]<<" ";
  VWCOUT<<endl;
}

void printchar(char* arr, int n) {
  for(int i = 0;i < n;i++)
    VWCOUT<<arr[i]<<" ";
  VWCOUT<<endl;
}

int sock_connect(uint32_t ip, int port) {

  int sock = socket(PF_INET, SOCK_STREAM, 0);
  if (sock == -1)
    {
      Rf_error("can't get socket ");
    }
  sockaddr_in far_end;
  far_end.sin_family = AF_INET;
  far_end.sin_port = port;

  far_end.sin_addr = *(in_addr*)&ip;
  memset(&far_end.sin_zero, '\0',8);
  if (connect(sock,(sockaddr*)&far_end, sizeof(far_end)) == -1)
    {
      VWCOUT << "can't connect to: " ;
      uint32_t pip = ntohl(ip);
      char* pp = (char*)&pip;
	
      for (size_t i = 0; i < 4; i++)
	{
	  VWCOUT << (int)pp[i] << ".";
	}
      VWCOUT << ':' << htons(port) << endl;
      Rf_error("");		// instead of perror(NULL)
      //exit(1);
    }
  return sock;
}

int getsock()
{
  int sock = socket(PF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    Rf_error("can't open socket!");
    }
    
    int on = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on)) < 0) 
      perror("setsockopt SO_REUSEADDR");
    return sock;
}

void all_reduce_init(string master_location, size_t unique_id, size_t total, size_t node)
{
  struct hostent* master = gethostbyname(master_location.c_str());
    
  if (master == NULL) {
    Rf_error("can't resolve hostname: %s", master_location.c_str());
  }
  current_master = master_location;

  uint32_t master_ip = * ((uint32_t*)master->h_addr);
  int port = 26543;
    
  int master_sock = sock_connect(master_ip, htons(port));

  if(write(master_sock, &unique_id, sizeof(unique_id)) < (int)sizeof(unique_id))
    VWCOUT << "write failed!" << endl; 
  if(write(master_sock, &total, sizeof(total)) < (int)sizeof(total))
    VWCOUT << "write failed!" << endl; 
  if(write(master_sock, &node, sizeof(node)) < (int)sizeof(node))
    VWCOUT << "write failed!" << endl; 
  int ok;
  if (read(master_sock, &ok, sizeof(ok)) < (int)sizeof(ok))
    VWCOUT << "read 1 failed!" << endl;
  if (!ok) {
    Rf_error("mapper already connected");
  }

  uint16_t kid_count;
  uint16_t parent_port;
  uint32_t parent_ip;

  if(read(master_sock, &kid_count, sizeof(kid_count)) < (int)sizeof(kid_count))
    VWCOUT << "read 2 failed!" << endl;

  int sock = -1;
  short unsigned int netport = htons(26544);
  if(kid_count > 0) {
    sock = getsock();
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = netport;

    bool listening = false;
    while(!listening)
      {
	if (::bind(sock,(sockaddr*)&address, sizeof(address)) < 0)
	  if (errno == EADDRINUSE)
	    {
	      netport = htons(ntohs(netport)+1);
	      address.sin_port = netport;
	    }
	  else
	    {
	      Rf_error("Bind failed ");
	    }
	else
	  if (listen(sock, kid_count) < 0)
	    {
	      perror("listen failed! ");
	      close(sock);
	      sock = getsock();
	    }
	  else
	    {
	      listening = true;
	    }
      }
  }
  
  if(write(master_sock, &netport, sizeof(netport)) < (int)sizeof(netport))
    VWCOUT << "write failed!" << endl;
  
  if(read(master_sock, &parent_ip, sizeof(parent_ip)) < (int)sizeof(parent_ip))
    VWCOUT << "read 3 failed!" << endl;
  if(read(master_sock, &parent_port, sizeof(parent_port)) < (int)sizeof(parent_port))
    VWCOUT << "read 4 failed!" << endl;

  
  close(master_sock);
    
  //int parent_sock;
  if(parent_ip != (uint32_t)-1) 
    socks.parent = sock_connect(parent_ip, parent_port);
  else
    socks.parent = -1;
    
  socks.children[0] = -1; socks.children[1] = -1;

  for (int i = 0; i < kid_count; i++)
    {
      sockaddr_in child_address;
      socklen_t size = sizeof(child_address);
      int f = accept(sock,(sockaddr*)&child_address,&size);
      if (f < 0)
	{
	  Rf_error("bad client socket!");
	}
      socks.children[i] = f;
    }

  if (kid_count > 0)
    close(sock);
}

void addbufs(float* buf1, float* buf2, int n) {
  for(int i = 0;i < n;i++) 
//     {
//       uint32_t first = *((uint32_t*)(buf1+i));
//       uint32_t second = *((uint32_t*)(buf2+i));
//       uint32_t xkindaor = first^second;
//       buf1[i] = *(float*)(&xkindaor);
//     }
    buf1[i] += buf2[i];
}


void pass_up(char* buffer, int left_read_pos, int right_read_pos, int& parent_sent_pos, int parent_sock, int n) {
  
  //cerr<<left_read_pos<<" "<<right_read_pos<<" "<<parent_sent_pos<<" "<<n<<endl;
  int my_bufsize = min(buf_size, ((int)(floor(left_read_pos/(sizeof(float)))*sizeof(float)) - parent_sent_pos));
  my_bufsize = min(my_bufsize, ((int)(floor(right_read_pos/(sizeof(float)))*sizeof(float)) - parent_sent_pos));

  if(my_bufsize > 0) {
    //going to pass up this chunk of data to the parent
    int write_size = write(parent_sock, buffer+parent_sent_pos, my_bufsize);
    if(write_size < my_bufsize) 
      VWCOUT<<"Write to parent failed "<<my_bufsize<<" "<<write_size<<" "<<parent_sent_pos<<" "<<left_read_pos<<" "<<right_read_pos<<endl ;
    parent_sent_pos += my_bufsize;
    //VWCOUT<<"buf size = "<<my_bufsize<<" "<<parent_sent_pos<<endl;
  }

}

void pass_down(char* buffer, int parent_read_pos, int&children_sent_pos, int* child_sockets, int n) {

  int my_bufsize = min(buf_size, (parent_read_pos - children_sent_pos));


  if(my_bufsize > 0) {
    //going to pass up this chunk of data to the children
    if(child_sockets[0] != -1 && write(child_sockets[0], buffer+children_sent_pos, my_bufsize) < my_bufsize) 
      VWCOUT<<"Write to left child failed\n";
    if(child_sockets[1] != -1 && write(child_sockets[1], buffer+children_sent_pos, my_bufsize) < my_bufsize) 
      VWCOUT<<"Write to right child failed\n";
    
    children_sent_pos += my_bufsize;
  }

}


void reduce(char* buffer, int n, int parent_sock, int* child_sockets) {

  fd_set fds;
  FD_ZERO(&fds);
  if(child_sockets[0] != -1)
    FD_SET(child_sockets[0],&fds);
  if(child_sockets[1] != -1)
    FD_SET(child_sockets[1],&fds);

  int max_fd = max(child_sockets[0],child_sockets[1])+1;
  int child_read_pos[2] = {0,0}; //First unread float from left and right children
  int child_unprocessed[2] = {0,0}; //The number of bytes sent by the child but not yet added to the buffer
  char child_read_buf[2][buf_size+sizeof(float)-1];
  int parent_sent_pos = 0; //First unsent float to parent
  //parent_sent_pos <= left_read_pos
  //parent_sent_pos <= right_read_pos
  
  if(child_sockets[0] == -1) {
    child_read_pos[0] = n;
  }
  if(child_sockets[1] == -1) {
    child_read_pos[1] = n;						 
  }

  while (parent_sent_pos < n || child_read_pos[0] < n || child_read_pos[1] < n)
    {
      if(parent_sock != -1)
	pass_up(buffer, child_read_pos[0], child_read_pos[1], parent_sent_pos, parent_sock, n);

      if(parent_sent_pos >= n && child_read_pos[0] >= n && child_read_pos[1] >= n) break;

      //      cout<<"Before select parent_sent_pos = "<<parent_sent_pos<<" child_read_pos[0] = "<<child_read_pos[0]<<" max fd = "<<max_fd<<endl;
      
      if(child_read_pos[0] < n || child_read_pos[1] < n) {
	if (max_fd > 0 && select(max_fd,&fds,NULL, NULL, NULL) == -1)
	  {
	    Rf_error("Select failed!");
	  }
      
	for(int i = 0;i < 2;i++) {
	  if(child_sockets[i] != -1 && FD_ISSET(child_sockets[i],&fds)) {
	    //there is data to be left from left child
	    if(child_read_pos[i] == n) {
	      VWCOUT<<"I think child has no data to send but he thinks he has "<<FD_ISSET(child_sockets[0],&fds)<<" "<<FD_ISSET(child_sockets[1],&fds)<<endl;
	      //fflush(stderr);
	      Rf_error("");
	    }
	  
	
	    //float read_buf[buf_size];
	    size_t count = min(buf_size,n - child_read_pos[i]);
	    int read_size = read(child_sockets[i], child_read_buf[i] + child_unprocessed[i], count);
	    if(read_size == -1) {
	      Rf_error("Read from child failed");
	    }
	    
	    //cout<<"Read "<<read_size<<" bytes\n";
// 	    char pattern[4] = {'A','B','C','D'};
// 	    for(int j = 0; j < (child_read_pos[i] + read_size)/sizeof(float) - child_read_pos[i]/sizeof(float);j++) {
// 	      if((buffer[(child_read_pos[i]/sizeof(float))*sizeof(float)+j] != pattern[j%4] && buffer[(child_read_pos[i]/sizeof(float))*sizeof(float)+j] != '\0') || (child_read_buf[i][j] != pattern[j%4]&& child_read_buf[i][j] != '\0')) {
// 		cerr<<"Wrong data "<<pattern[j%4]<<" "<<buffer[(child_read_pos[i]/sizeof(float))*sizeof(float)+j]<<" "<<child_read_buf[i][j]<<endl;
// 		cerr<<"Reading from positions "<<child_read_pos[i]/sizeof(float)+j<<" "<<j<<" "<<child_unprocessed[i]<<" "<<child_read_pos[i]<<endl;
// 		cerr<<"Reading from child "<<i<<" "<<child_read_pos[0]<<" "<<child_read_pos[1]<<endl;
// 	      }
// 	    }

	    addbufs((float*)buffer + child_read_pos[i]/sizeof(float), (float*)child_read_buf[i], (child_read_pos[i] + read_size)/sizeof(float) - child_read_pos[i]/sizeof(float));
	    
	    child_read_pos[i] += read_size;
	    int old_unprocessed = child_unprocessed[i];
	    child_unprocessed[i] = child_read_pos[i] % (int)sizeof(float);
	    //cout<<"Unprocessed "<<child_unprocessed[i]<<" "<<(old_unprocessed + read_size)%(int)sizeof(float)<<" ";
	    for(int j = 0;j < child_unprocessed[i];j++) {
	      // cout<<(child_read_pos[i]/(int)sizeof(float))*(int)sizeof(float)+j<<" ";
	      child_read_buf[i][j] = child_read_buf[i][((old_unprocessed + read_size)/(int)sizeof(float))*sizeof(float)+j];
	    }
	    //cout<<endl;
	  
	    if(child_read_pos[i] == n) //Done reading parent
	      FD_CLR(child_sockets[i],&fds);
	    //cerr<<"Clearing socket "<<i<<" "<<FD_ISSET(child_sockets[i],&fds)<<endl;
	    //cerr<<"Child_unprocessed should be 0"<<child_unprocessed[i]<<endl;
	    //fflush(stderr);
	  }
	  else if(child_sockets[i] != -1 && child_read_pos[i] != n)
	    FD_SET(child_sockets[i],&fds);      
	}
      }
      if(parent_sock == -1 && child_read_pos[0] == n && child_read_pos[1] == n) 
	parent_sent_pos = n;

    }  
  
}

void broadcast(char* buffer, int n, int parent_sock, int* child_sockets) {

 
   int parent_read_pos = 0; //First unread float from parent
   int children_sent_pos = 0; //First unsent float to children
  //parent_sent_pos <= left_read_pos
  //parent_sent_pos <= right_read_pos
  
   if(parent_sock == -1) {
     parent_read_pos = n;						 
   }
   if(child_sockets[0] == -1 && child_sockets[1] == -1) 
     children_sent_pos = n;

   while (parent_read_pos < n || children_sent_pos < n)
    {
      pass_down(buffer, parent_read_pos, children_sent_pos, child_sockets, n);
      if(parent_read_pos >= n && children_sent_pos >= n) break;

      if (parent_sock != -1) {
	//there is data to be read from the parent
	if(parent_read_pos == n) {
	  Rf_error("I think parent has no data to send but he thinks he has");
	}
	size_t count = min(buf_size,n-parent_read_pos);
	int read_size = read(parent_sock, buffer + parent_read_pos, count);
	if(read_size == -1) {
	  Rf_error("Read from parent failed");
	}
	parent_read_pos += read_size;	
      }
    }
}

void all_reduce(char* buffer, int n, string master_location, size_t unique_id, size_t total, size_t node) 
{
  if(master_location != current_master) 
    all_reduce_init(master_location, unique_id, total, node);
    
  reduce(buffer, n, socks.parent, socks.children);
  broadcast(buffer, n, socks.parent, socks.children);
}

node_socks::~node_socks()
{
  if(current_master != "") {
    if(this->parent != -1)
      close(this->parent);
    if(this->children[0] != -1) 
      close(this->children[0]);
    if(this->children[1] != -1)
      close(this->children[1]);  
  }
}

