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
  #include <ctype.h>
  
 #define LINESIZE 81

 char *inBuff;
 char name[12]="Csvfile";
 char *achOK="OK";
 int nId,nNum;
 int nSize;
 
 char *ipAddress = NULL;
 char *file=NULL;
 char *argName=NULL;
 int col;
 int skip;
 int rew;
 int bEof;
 int numSocket;
 int SocketFD;

 FILE *fp=NULL;
 char *lineBuf;
 size_t bufSize=LINESIZE;
 
 double *dataBuf=NULL;
 double time;
 
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

int setupFile()
{
  if ((file==NULL)||(fp!=NULL))
    return 0;
  fp=fopen(file,"r");
  bEof=0;
  return (fp!=NULL);
}

int readLine()
{
  float d;
  int i;
  char ch;
  int ret;
  char *s1;
  char *s2;
  
  if (fp==NULL)
      return 0;
  if (getline(&lineBuf,&bufSize,fp)<0)
  {
    perror("getline: ");
      return 0;
  }
  //skip columns if applicable
  if (col)
  {
    for (i=0;i<col;i++)
      sscanf(lineBuf,"%f,",&d);
  }
  //printf("Read line: %s\n",lineBuf);
  //read in variables, it is up to the user to make sure the correct number is called for:
  i=0;
  s2=lineBuf;
  do {
      //printf("Do s2=%s\n",s2);
          while( (*s2 == ' ') || (*s2 == '\t') )  s2++;
           s1 = strsep( &s2, "," );
      //printf("Do s1=%s\n",s1);
           if( *s1 )
	   {
            ret = sscanf( s1, " %f %c", &d, &ch );
            if( ret == 1 )
	    {
	    //  printf("data point %d=%f %c",i,d,ch);
	      dataBuf[i]=d;
            }
	   }
            i++;
	    if (i==nNum) //break
	      s2=0;
        } while (s2!=0 );
  return 1;
}

void streamData()
{ 
    float step;
    int i,j,len,n;
    int bStop=0;

    /* perform read write operations ... */
    while(!bStop)
    {
     memset(inBuff, 0, nSize);
     len=read(SocketFD,inBuff,nSize);
     if (len>0)
     {
      for (j=0;j<len;j++)
      {
       switch(inBuff[j])
       {
        default:
	{
	  printf("UNRECOGNISED STRING: %s\n",inBuff);
	  len=0;
	  break;
	}
	case 'E':
	{
	  //print program name and version
	  printf("%s\n",inBuff);
	  len=0;
	  break;
	}
        case '[':
        {
	 //printf("Got id string: %s\n",inBuff);
	 sscanf(inBuff,"[%d,%d,%f,%d]",&nId,&nNum,&step,&n);
	 if (isnan(step)||(step==0.0))
	   step=0.01;
	 printf("Found id %d\nNumber of streams: %d\nStep size: %f\nBuffer size: %d\n",nId,nNum,step,n);
	 if (n!=nSize)
	 {
	   if (inBuff!=NULL)
	     free(inBuff);
	   inBuff=malloc(n+1);
	   memset(inBuff, 0, n+1);
	   nSize=n;
	 }
	 //allocate data storage
	 if (dataBuf!=NULL)
	   free(dataBuf);
	 dataBuf=(double *)malloc(nNum*sizeof(double));
	 
         write(SocketFD,achOK,strlen(achOK));	
	 fsync(SocketFD);
	 len=0;
 	 break;
        }
        case 'H': 
        {
 	 printf("Got hello, sending name\n");
         write(SocketFD,name,sizeof(name));
	 fsync(SocketFD);
	 break;
        }
        case 'P':
        {
	 printf("Got stop, exiting...\n");
	 bStop=1;
	 break;
       }
       case 'T':
	{
     // printf("{%s}\n",inBuff);
	 n=sscanf(inBuff,"T%f",&step);
// 	 printf("Found step size %f (%d){%s}\n",step,n,inBuff);
	 if (isnan(step)||(step==0.0))
	   step=0.01;
	 printf("Found step size %f\n",step);
  	 len=0;
	 break;
	}
       case 'S':
       {
	      time=0.0;
	      if (readLine())
	      {
		for (i=0;i<nNum;i++)
		{
		  write(SocketFD,&nId,sizeof(nId));
		  write(SocketFD,&i,sizeof(i));
		  write(SocketFD,&time,sizeof(time));
		  write(SocketFD,&dataBuf[i],sizeof(double));
		}
		printf("Started writing data\n");
		fsync(SocketFD);
	      }
	      else
	      {
		printf("Error in readline, exiting...\n");
		bStop=1;
	      }
	      break;
       }
       case 'N':
       {
	      time=time+step;
	      if (readLine())
	      {
		for (i=0;i<nNum;i++)
		{
		  write(SocketFD,&nId,sizeof(nId));
		  write(SocketFD,&i,sizeof(i));
		  write(SocketFD,&time,sizeof(time));
		  write(SocketFD,&dataBuf[i],sizeof(double));
		  printf("Written data (%f) %d=%f \n",time,i,dataBuf[i]);
		}
		fsync(SocketFD);
	      }
	      else
	      {
		if (!bEof)
		{
		 printf("Error in readline, exiting...\n");
	    	 bStop=0;
		}
	      }
	      //test for end of file
	      if (feof(fp))
	      {
		if (rew)
		{
		  rewind(fp);
		  bEof=0;
		}
		else
		  bEof=1;
	      }
	      break;
	 }
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
    int c,i;
    
    opterr = 0;
    numSocket=0;
    col=0;
    skip=0;
    rew=1;
    nSize=1024;
    inBuff=malloc(nSize+1);
    memset(inBuff, 0, sizeof(inBuff));
    time=0.0;
     
    while ((c = getopt (argc, argv, "i:s:f:c:k:n:hr")) != -1)
       {
         switch (c)
           {
	   case 'h':
	     printUsage();
	     break;
	   case 'r':
	     rew=0;
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
    if (!setupSocket()||(!setupFile()))
    {
      exit(EXIT_FAILURE);
    }
    //alloc line buffer
    lineBuf=malloc(LINESIZE);
    if (lineBuf==NULL)
    {
      exit(EXIT_FAILURE);
    }
    memset(lineBuf, 0, sizeof(lineBuf));
    //if skip a number of lines
    for (i=0;i<skip;i++)
      readLine();    
    //output data:
    streamData();
    
    shutdown(SocketFD, SHUT_RDWR);
    free(lineBuf);
    close(SocketFD);
    fclose(fp);
    return EXIT_SUCCESS;
}    
