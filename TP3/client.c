#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include "functions.h"

#define BUFLEN 512
#define PORT 9939
#define FILE_NAME "navet.jpg"
#define FILE_RECEP "recep.jpg"

#define SRV_IP "127.0.0.1"

int main(void)
{
  struct sockaddr_in SA_ser_con, SA_ser_data; //to make the connexion ; to send data
  int s_con, s_data, slen_con=sizeof(SA_ser_con), slen_data=sizeof(SA_ser_data);
  socklen_t sLen = sizeof(SA_ser_con);
  socklen_t sLen_data = sizeof(SA_ser_data);
  char buf[BUFLEN];
  int PORT_DATA;
  FILE* frecep;
  //int buf_index;
  int seq_nb, last_seq_nb, last_buf_size;
  char seq_nb_char[6]; //seq_nb in a character array

  if ((s_con=socket(AF_INET, SOCK_DGRAM, 0))==-1)
    diep("socket");
  if ((s_data=socket(AF_INET, SOCK_DGRAM, 0))==-1)
    diep("socket");

  memset((char *) &SA_ser_con, 0, sizeof(SA_ser_con));
  SA_ser_con.sin_family = AF_INET;
  SA_ser_con.sin_port = htons(PORT);
  if (inet_aton(SRV_IP, &SA_ser_con.sin_addr)==0) {
    fprintf(stderr, "inet_aton() failed\n");
    exit(1);
  }

  memset((char *) &SA_ser_data, 0, sizeof(SA_ser_data));
  SA_ser_data.sin_family = AF_INET;
  //give the port when we receive it drom the server in "etablishment of a connexion"
  if (inet_aton(SRV_IP, &SA_ser_data.sin_addr)==0) {
    fprintf(stderr, "inet_aton() failed\n");
    exit(1);
  }


/*******************************/
/*etablishment of a connexion**/
  sprintf(buf, "SYN");
  printf("Sending packet %s\n", buf);
  if (sendto(s_con, buf, BUFLEN, 0, (struct sockaddr*)&SA_ser_con, slen_con)==-1)
    diep("sendto()");

  init(buf, BUFLEN);
  if(recvfrom(s_con, buf, BUFLEN, 0, (struct sockaddr*)&SA_ser_con, &sLen)==-1)
    diep("recvfrom SYN-ACK");
  display(buf, BUFLEN);
  PORT_DATA = get_port(buf, strlen(buf));//get the port for the data exchanges
  printf("Port for data from server : %d\n", PORT_DATA);
  SA_ser_data.sin_port = htons(PORT_DATA);//give the port nb we receive to the SA
  init(buf, BUFLEN);
  sprintf(buf, "ACK");
  printf("Sending packet %s\n", buf);
  if (sendto(s_con, buf, BUFLEN, 0, (struct sockaddr *)&SA_ser_con, slen_con)==-1)
    diep("sendto() ACK");
  init(buf, BUFLEN);
/***************************************/


/**************************************/
/*file transfert*************************/
  //file demand
  init(buf, BUFLEN);
  sprintf(buf, FILE_NAME);
  frecep = fopen(FILE_RECEP, "wb");
  if (sendto(s_data, buf, BUFLEN, 0, (struct sockaddr*)&SA_ser_data, slen_data)==-1)
    diep("sendto() file demand");
  printf("sending %s\n", buf);

  init(buf, BUFLEN);
  if (recvfrom(s_data, buf, BUFLEN, 0, (struct sockaddr*)&SA_ser_data, &sLen_data)==-1)
    diep("recvfrom() last_seq_nb");
  printf("I received the last_seq_nb : %s\n", buf);
  last_seq_nb = atoi(buf);
  init(buf, BUFLEN);
  sprintf(buf, "ACK_last_seq_nb");
  //if (sendto(s_con, buf, BUFLEN, 0, (struct sockaddr*)&SA_ser_con, sizeof(SA_ser_con))==-1)
  //  diep("sendto() ack last_seq_nb");
  init(buf, BUFLEN);
  if (recvfrom(s_data, buf, BUFLEN, 0, (struct sockaddr*)&SA_ser_data, &sLen_data)==-1)
    diep("recvfrom() last_buf_size");
  printf("I received the last_buf_size : %s\n", buf);
  last_buf_size = atoi(buf);

  //file reception
  seq_nb = 1;
  printf("the last_seq_nb is : %d\n", last_seq_nb);
  while(seq_nb<last_seq_nb){
    init(buf, BUFLEN);
    if (recvfrom(s_data, buf, BUFLEN, 0, (struct sockaddr*)&SA_ser_data, &sLen_data)==-1)
      diep("recvfrom() file");
    //printf ("I received : %s\n", buf);
    fwrite(buf, sizeof(char), BUFLEN-1, frecep); //-1 because last char of buf is /0
    init(buf, BUFLEN);
    sprintf(buf, "ACK_");
    init(seq_nb_char, 6);
    sprintf(seq_nb_char, "%d", seq_nb);
    strcat(buf, seq_nb_char);
    seq_nb++;
    if (sendto(s_data, buf, BUFLEN, 0, (struct sockaddr*)&SA_ser_data, sizeof(SA_ser_data))==-1)
      diep("ACK data");
    printf("I sent the ack : %s\n", buf);
  }
  //last buf
  init(buf, BUFLEN);
  if (recvfrom(s_data, buf, BUFLEN, 0, (struct sockaddr*)&SA_ser_data, &sLen_data)==-1)
    diep("recvfrom() file");
    //printf ("I received : %s\n", buf);
  fwrite(buf, sizeof(char), last_buf_size, frecep); 
  init(buf, BUFLEN);
  sprintf(buf, "ACK_");
  init(seq_nb_char, 6);
  sprintf(seq_nb_char, "%d", seq_nb);
  strcat(buf, seq_nb_char);
  seq_nb++;
  if (sendto(s_data, buf, BUFLEN, 0, (struct sockaddr*)&SA_ser_data, sizeof(SA_ser_data))==-1)
    diep("ACK data");
  printf("I sent the ack : %s\n", buf);



  fclose(frecep);

/******************************************/

  close(s_con);
  close(s_data);
  return 0;

}
