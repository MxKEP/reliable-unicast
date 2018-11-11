#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include "udppacket.h"

#define TIMEOUT_SECS    2
#define RETRANSMIT   10
const int MAXSIZE = 4096;
const int BYTESIZE = 500;

int tries = 0, base = 0, windowSize = 0, sendflag = 1;

int main (int argc, char ** argv) {

  FILE *fp;
  char filebuffer[MAXSIZE];
  fp = fopen("beforeTransfer.txt", "r");
  int sock, reclen, packet_received = -1, packet_sent = -1, numpackets = 0;
  struct sockaddr_in localaddr, serveraddr;
  unsigned short port;
  unsigned int servsize;
  struct sigaction event;
  char *servIP;
  char buffer[MAXSIZE+1];
  const int datasize = 4096;

  if (argc != 3){
      fprintf (stderr,"Command Line args : %s <Server IP> <Server Port No>\n", argv[0]);
      exit (1);
    }

  servIP = argv[1];
  port = atoi (argv[2]);
  windowSize = 2;

  if(fp == NULL)
  printf("FILE DOES NOT EXIST\n");

  while(fgets(filebuffer, MAXSIZE, fp))
          strcat(buffer, filebuffer);

      fclose(fp);

  if(BYTESIZE >= 512)
  {
    fprintf(stderr, "chunk size must be less than 512\n");
    exit(1);
  }

  numpackets = datasize / BYTESIZE;
  if (datasize % BYTESIZE)
    numpackets++;

  if ((sock = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    error_close ("socket() failed");

  event.sa_handler = timeout_exit;
  if (sigfillset (&event.sa_mask) < 0)
    error_close ("sigfillset() failed");
  event.sa_flags = 0;

  if (sigaction (SIGALRM, &event, 0) < 0)
    error_close ("sigaction() failed for SIGALRM");

  memset (&localaddr, 0, sizeof (localaddr));
  localaddr.sin_family = AF_INET;
  localaddr.sin_addr.s_addr = inet_addr (servIP);
  localaddr.sin_port = htons (port);

  while ((packet_received < numpackets-1) && (tries < RETRANSMIT))
    {
      if (sendflag > 0)
	{
	sendflag = 0;
	  int i;
	  for (i = 0; i < windowSize; i++)
	    {
	      packet_sent = min(max (base + i, packet_sent),numpackets-1);
	      struct packet current;
	      if ((base + i) < numpackets)
		{
		  memset(&current,0,sizeof(current));
 		  printf ("Transmitting packet : %d Received packet : %d\n",
                      base+i, packet_received);

		  current.type = htonl (1);
		  current.seq_no = htonl (base + i);
		  int currlength;
		  if ((datasize - ((base + i) * BYTESIZE)) >= BYTESIZE)
		    currlength = BYTESIZE;
		  else
		    currlength = datasize % BYTESIZE;
		  current.length = htonl (currlength);
		  memcpy (current.data, buffer + ((base + i) * BYTESIZE), currlength);
		  if (sendto(sock, &current, (sizeof (int) * 3) + currlength, 0,(struct sockaddr *) &localaddr,sizeof (localaddr)) !=((sizeof (int) * 3) + currlength))
		    error_close
		      ("sendto() sent a different number of bytes than expected");
		}
	  }
	}
      servsize = sizeof (serveraddr);
      alarm (TIMEOUT_SECS);	/* Set the timeout */
      struct packet currAck;
      while ((reclen = (recvfrom (sock, &currAck, sizeof (int) * 3, 0,
				   (struct sockaddr *) &serveraddr,
				   &servsize))) < 0)
	if (errno == EINTR)	/* Alarm went off  */
	  {
	    if (tries < RETRANSMIT)	/* incremented by signal handler */
	      {
		printf ("%d more transmissions until timeout :\n", RETRANSMIT - tries);
		break;
	      } else
	      error_close ("connection failed");
	  }else
	  error_close ("recvfrom() failed");

      if (reclen){
	  int acktype = ntohl (currAck.type);
	  int ackno = ntohl (currAck.seq_no);
	  if (ackno > packet_received && acktype == 2) {
	      printf ("Received ACK from server\n");
	      packet_received++;
	      base = packet_received;
	      if (packet_received == packet_sent)
		{
		  alarm (0);
		  tries = 0;
		  sendflag = 1;
		}
	      else
		{
		  tries = 0;
		  sendflag = 0;
		  alarm(TIMEOUT_SECS);
		}
	    }
	}
    }
  int i;
  for (i = 0; i < 10; i++)
    {
      struct packet closing;
      closing.type = htonl (4);
      closing.seq_no = htonl (0);
      closing.length = htonl (0);
      sendto (sock, &closing, (sizeof (int) * 3), 0,
	      (struct sockaddr *) &localaddr, sizeof (localaddr));
    }
  close (sock);
  fclose(fp);
  exit (0);
}

void timeout_exit (int exit)
{
  tries += 1;
  sendflag = 1;
}

void error_close (char *errorMessage)
{
  perror (errorMessage);
  exit (1);
}

int max (int x, int y)
{
  if (y > x)
    return y;
  return x;
}

int min(int x, int y)
{
  if( y > x )
	return x;
  return y;
}
