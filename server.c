#include  <stdlib.h>
#include  <unistd.h>
#include  <stdio.h>
#include  <sys/socket.h>
#include  <arpa/inet.h>
#include  <string.h>
#include  <errno.h>
#include  <netinet/in.h>
#include  <sys/types.h>
#include <signal.h>
#include <netdb.h>
#include "udppacket.h"
const int MAXSIZE = 4096;
const int BYTESIZE = 500;

int main (int argc, char ** argv){

  FILE *fp;
  fp = fopen("afterTransfer.txt", "w");
  char buffer[MAXSIZE];
  buffer[MAXSIZE] = '\0';
  int sock, pcksize, rec_packet = -1;
  struct sockaddr_in localaddr;
  struct sockaddr_in clientaddr;
  unsigned int cliAddrLen;
  unsigned short port;
  struct sigaction event;
  double lossRate;

  bzero (buffer, MAXSIZE);
  if (argc < 2 || argc > 3){
      fprintf (stderr, "commant line args :  %s <PORT> [<LOSS PERCENTAGE DECIMAL(0.0 - .99)>]\n", argv[0]);
      exit (1);
    }

  port = atoi (argv[1]);
  lossRate = atof(argv[2]);
  srand48(192837465);
  if ((sock = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    error_close ("socket() failed");

  memset (&localaddr, 0, sizeof (localaddr));
  localaddr.sin_family = AF_INET;
  localaddr.sin_addr.s_addr = htonl (INADDR_ANY);
  localaddr.sin_port = htons (port);

  if (bind (sock, (struct sockaddr *) &localaddr, sizeof (localaddr)) < 0)
    error_close ("bind() failed");
  event.sa_handler = timeout_exit;
  if (sigfillset (&event.sa_mask) < 0)
  event.sa_flags = 0;
  if (sigaction (SIGALRM, &event, 0) < 0)
    error_close ("failed");
  for (  ;  ;  ){
      cliAddrLen = sizeof (clientaddr);
      struct packet currPacket;
      memset(&currPacket, 0, sizeof(currPacket));
      pcksize = recvfrom (sock, &currPacket, sizeof (currPacket), 0, (struct sockaddr *) &clientaddr, &cliAddrLen);
      currPacket.type = ntohl (currPacket.type);
      currPacket.length = ntohl (currPacket.length);
      currPacket.seq_no = ntohl (currPacket.seq_no);

      if (currPacket.type == 4)
	{
      fputs(buffer, fp);
      fclose(fp);
	  struct packet ackpacket;
	  ackpacket.type = htonl(8);
	  ackpacket.seq_no = htonl(0);
	  ackpacket.length = htonl(0);
	  if (sendto
	      (sock, &ackpacket, sizeof (ackpacket), 0,
	       (struct sockaddr *) &clientaddr,
	       cliAddrLen) != sizeof (ackpacket))
	    {
	      error_close ("Error");
      }
	  alarm (7);
	  while (1)
	    {
	      while ((recvfrom (sock, &currPacket, sizeof (int)*3+BYTESIZE, 0,
				(struct sockaddr *) &clientaddr,
				&cliAddrLen))<0)
		{
		  if (errno == EINTR)
		    {
		      exit(0);
		    }

		}
	      if (ntohl(currPacket.type) == 4)
		{
		  ackpacket.type = htonl(8);
		  ackpacket.seq_no = htonl(0);
		  ackpacket.length = htonl(0);
		  if (sendto(sock, &ackpacket, sizeof (ackpacket), 0,(struct sockaddr *) &clientaddr, cliAddrLen) != sizeof (ackpacket))
		    {
		      error_close ("Error sending tear-down ack");
		    }
		}
	 }
	  error_close ("recvfrom() failed");
	}else{
	     if(lossRate > drand48())
		continue;
	  printf (":: RECEIVE PACKET %d -- %d bytes\n", currPacket.seq_no, currPacket.length);

	  if (currPacket.seq_no == rec_packet + 1)
	    {
	      rec_packet++;
	      int buff_offset = BYTESIZE * currPacket.seq_no;
	      memcpy (&buffer[buff_offset], currPacket.data,
		      currPacket.length);
	    }
	  printf (":: SEND ACK %d\n", rec_packet);
	  struct packet current;
	  current.type = htonl (2);
	  current.seq_no = htonl (rec_packet);
	  current.length = htonl(0);
	  if (sendto (sock, &current, sizeof (current), 0, /* send ack */(struct sockaddr *) &clientaddr,cliAddrLen) != sizeof (current))
	    error_close
	      ("sendfailed");
	}
  }
}

void error_close (char *errorMessage){
  perror (errorMessage);
  exit (1);
}

void timeout_exit (int x) /* just exit when encounter timeout */
{
  exit(0);
}
