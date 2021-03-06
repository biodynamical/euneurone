 /* Client code in C */
 
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <unistd.h>
  #include <math.h>
 
#define _DEBUG

 char buff[81];
 char name[12]="Sinewave";
 
  int main(void)
  {
    struct sockaddr_in stSockAddr;
    int Res;
    int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
 
    if (-1 == SocketFD)
    {
      perror("cannot create socket");
      exit(EXIT_FAILURE);
    }
 
    memset(&stSockAddr, 0, sizeof(stSockAddr));
 
    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(5555);
    Res = inet_pton(AF_INET, "161.73.147.55", &stSockAddr.sin_addr);
 
    if (0 > Res)
    {
      perror("error: first parameter is not a valid address family");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }
    else if (0 == Res)
    {
      perror("char string (second parameter does not contain valid ipaddress)");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }
 
    if (-1 == connect(SocketFD, (struct sockaddr *)&stSockAddr, sizeof(stSockAddr)))
    {
      perror("connect failed");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }
 
    memset(&buff, 0, sizeof(buff));
 
    double x,y;
    int bStop=0;
    x=0.0;
    y=0.0;
    /* perform read write operations ... */
    read(SocketFD,buff,80);
    printf("Read name: %s\n",buff);
    write(SocketFD,name,sizeof(name));
    while(!bStop)
    {
     memset(&buff, 0, sizeof(buff));
     if (read(SocketFD,buff,80)>0)
     {
#ifdef _DEBUG
//	printf("Read:%s\n",buff);
#endif
     }
     switch(buff[0])
     {
       case '[':
       {
#ifdef _DEBUG
	printf("Got id string: %s\n",buff);
#endif
	break;
       }
       case 'H': 
       {
#ifdef _DEBUG
	printf("Got hello\n");
#endif
	break;
       }
       case 'P':
       {
#ifdef _DEBUG
	 printf("Got stop, exiting...\n");
#endif
	 bStop=1;
	 break;
       }
       case 'S':
       {
	      x=0;
	      y=sin(x);
	      write(SocketFD,&y,sizeof(y));
#ifdef _DEBUG
	      printf("Start write: %f\n",y);
#endif
	      fsync(SocketFD);
	      break;
       }
       case 'N':
       {
	      x=x+0.1;
	      y=sin(x);
	      write(SocketFD,&y,sizeof(y));
#ifdef _DEBUG
//	      printf("Write: %f\n",y);
#endif
	      fsync(SocketFD);
	      break;
       }
     }
    }
    shutdown(SocketFD, SHUT_RDWR);
 
    close(SocketFD);
    return EXIT_SUCCESS;
  }
