 /* Euneurone client sample code */
 // gcc TestClientSin.c -lm -o TestClientSin
 
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <unistd.h>
  #include <math.h>
 
 char buff[81];
 char name[12]="Csvfile";
 char *ipAddress = NULL;
 char *file=NULL;
 char *argName=NULL;
 int col;
 int skip;
 int numSocket;
 int SocketFD;
 
int setupSocket()
{
    struct sockaddr_in stSockAddr;
    int Res; 

    SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
 
    if (-1 == SocketFD)
    {
      perror("cannot create socket");
      return 0;
    }
   memset(&stSockAddr, 0, sizeof(stSockAddr));
 
    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(numSocket);
    Res = inet_pton(AF_INET, ipAddress, &stSockAddr.sin_addr);
 
    if (0 > Res)
    {
      perror("error: first parameter is not a valid address family");
      close(SocketFD);
      return 0;
    }
    else if (0 == Res)
    {
      perror("char string (second parameter does not contain valid ipaddress)");
      close(SocketFD);
      return 0;
    }
 
    if (-1 == connect(SocketFD, (struct sockaddr *)&stSockAddr, sizeof(stSockAddr)))
    {
      perror("connect failed");
      close(SocketFD);
      return 0;
    }
  return 1;
}

void streamData()
{ 
    double x,y;
    int bStop=0;
    x=0.0;
    y=0.0;

    /* perform read write operations ... */
    while(!bStop)
    {
     memset(&buff, 0, sizeof(buff));
     if (read(SocketFD,buff,80)>0)
     {
      switch(buff[0])
      {
       default:break;
       case '[':
       {
	printf("Got id string: %s\n",buff);
	break;
       }
       case 'H': 
       {
	printf("Got hello, sending name\n");
        write(SocketFD,name,sizeof(name));
	break;
       }
       case 'P':
       {
	 printf("Got stop, exiting...\n");
	 bStop=1;
	 break;
       }
       case 'S':
       {
	      x=0;
	      y=cos(x);
	      write(SocketFD,&y,sizeof(y));
	      printf("Start write: %f\n",y);
	      fsync(SocketFD);
	      break;
       }
       case 'N':
       {
	      x=x+0.1;
	      y=cos(x);
	      write(SocketFD,&y,sizeof(y));
	      fsync(SocketFD);
	      break;
       }
      }
     }
    }
 }

 /* Write out how to use this client */
 void printUsage()
 {
 }
 
int main(int argc, char**argv)
  {
    int c;
    
    opterr = 0;
    numSocket=0;
    col=0;
    skip=0;
    memset(&buff, 0, sizeof(buff));
     
    while ((c = getopt (argc, argv, "i:s:f:c:k:n:h")) != -1)
       {
         switch (c)
           {
	   case 'h':
	     printUsage();
	     break;
	   case 'c':
	     col=atoi(optarg);
	     break;
	   case 'k':
	     skip=atoi(optarg);
	     break;
	   case 'f':
	     file=optarg;
	     break;
           case 's':
             numSocket = atoi(optarg);
             break;
           case 'n':
             argName = optarg;
             break;
           case 'i':
             ipAddress = optarg;
             break;
           case '?':
             if ((optopt == 'i')||(optopt == 's')||(optopt == 'f')||(optopt == 'c')||(optopt == 'k')||(optopt == 'n'))
               fprintf (stderr, "Option -%c requires an argument.\n", optopt);
             else if (isprint (optopt))
               fprintf (stderr, "Unknown option `-%c'.\n", optopt);
             else
               fprintf (stderr,"Unknown option character `\\x%x'.\n",optopt);
             return 1;
           default:
             abort ();
           }
       }
       
    printf ("Euneurone Socket Client\nName: %s\nIP address: %s\nSocket %d\n",name,ipAddress,numSocket);
    
    if (ipAddress==NULL)
    {
      printf("Please provide IP address: e.g. -i 127.0.0.1\n");
      exit(-1);
    }
    if (file==NULL)
    {
      printf("Please provide file name: e.g. -f test.csv\n");
      exit(-1);
    }
    if (numSocket==0)
    {
      printf("Please provide socket: e.g. -s 1234\n");
      exit(-1);
    }
    //setup socket
    if (!setupSocket())
    {
      exit(EXIT_FAILURE);
    }
    //output data:
    streamData();
    
    shutdown(SocketFD, SHUT_RDWR);
 
    close(SocketFD);
    return EXIT_SUCCESS;
}    
