#ifndef udppacket_h
#define udppacket_h

struct packet
{
  int type;
  int seq_no;
  int length;
  char data[512];
};

void error_close (char *errorMessage);
void timeout_exit (int ignored);
int max (int a, int b);
int min(int a, int b);		
#endif
