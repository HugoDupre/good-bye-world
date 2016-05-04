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
#define PORT_CON 9939
#define PORT_DATA "5555"

int main(void)
{
  int seq_nb_size = 6;
  int BUFTEMPLEN = BUFLEN - seq_nb_size;
  struct sockaddr_in SA_me_con, SA_me_data, SA_client;//to listen to the port conexion ; listen to the data port ; get info from client in recvfrom
  int s_con, s_data;//socket for connexion ; data
  socklen_t sLen=sizeof(SA_client);
  char buf[BUFLEN];
  char buf_temp[BUFLEN-6];
  char seq_nb_char[6];
  FILE* fsend;
  int file_size, last_seq_nb, seq_nb, last_buf_size;

  if ((s_con=socket(AF_INET, SOCK_DGRAM, 0))==-1)
    diep("socket connexion");
  if ((s_data=socket(AF_INET, SOCK_DGRAM, 0))==-1)
    diep("socket data");

  memset((char *) &SA_me_con, 0, sizeof(SA_me_con));
  SA_me_con.sin_family = AF_INET;
  SA_me_con.sin_port = htons(PORT_CON);//port used to create the connexion
  SA_me_con.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(s_con, (struct sockaddr*)&SA_me_con, sizeof(SA_me_con))==-1)//SA with the PORT_CON
      diep("bind");

  memset((char *) &SA_me_data, 0, sizeof(SA_me_data));
  SA_me_data.sin_family = AF_INET;
  SA_me_data.sin_port = htons(atoi(PORT_DATA));//port used to exchange data
  SA_me_data.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(s_data, (struct sockaddr*)&SA_me_data, sizeof(SA_me_data))==-1)//name the socket s_data
      diep("bind");

/**************************************/
/*opening of the connecton : SYN steps*/
  memset((char*)&SA_client, 0, sizeof(SA_client));
  if (recvfrom(s_con, buf, BUFLEN, 0, (struct sockaddr*)&SA_client, &sLen)==-1)
    diep("recvfrom()");
  printf("Received packet from %s : %d\nData: %s\n\n", inet_ntoa(SA_client.sin_addr), ntohs(SA_client.sin_port), buf);
  
  if (strcmp(buf, "SYN")==0){
    init(buf, BUFLEN);
    sprintf(buf, "SYN_ACK");
    strcat(buf, PORT_DATA);
    display(buf, BUFLEN);

    printf("Sending packet %s\n", buf);
    if(sendto(s_con, buf, BUFLEN, 0, (struct sockaddr*)&SA_client, sizeof(SA_client))==-1)
      diep("sendto() SYN-ACK");
  }
  init(buf, BUFLEN);
  if (recvfrom(s_con, buf, BUFLEN, 0, (struct sockaddr*)&SA_client, &sLen)==-1)
    diep("recvfrom() ACK");
  display(buf, BUFLEN);
/*                                         */
/*******************************************/


/****************************/
/*file transfert************/
  init(buf, BUFLEN);
  if (recvfrom(s_data, buf, BUFLEN, 0, (struct sockaddr*)&SA_client, &sLen)==-1)
    diep("recvfrom() file name");
  printf("I received the name of the file : %s\n", buf);

  fsend = fopen(buf, "rb");
  if(fsend==NULL) {printf("error opening file\n"); exit(-1);}
  if(fseek(fsend, 0, SEEK_END)!=0)
    printf("error in fseek");
  file_size = ftell(fsend);//get the size of the file
  printf("the size of the file is %d bytes\n", file_size);

  last_seq_nb = (int)(file_size/(BUFLEN-1)) + 1;//-1 for /0 in last char ; +1 for the last non full buffer sent
  last_buf_size = file_size%(BUFLEN-1);
  init(buf, BUFLEN);
  sprintf(buf, "%d", last_seq_nb);
  if (sendto(s_data,  buf, BUFLEN, 0, (struct sockaddr*)&SA_client, sizeof(SA_client))==-1)
    diep("sendto() last_seq_nb");

  init(buf, BUFLEN);
  //if (recvfrom(s_con, buf, BUFLEN, 0, (struct sockaddr*)&SA_client, &sLen)==-1)
  //  diep("recvfrom() ack for the last_seq_nb");
  //printf("I received the ack last_seq_nb : %s\n", buf);
  sprintf(buf, "%d", last_buf_size);
  if (sendto(s_data,  buf, BUFLEN, 0, (struct sockaddr*)&SA_client, sizeof(SA_client))==-1)
    diep("sendto() last_buf_size");


  if (fseek(fsend, 0, SEEK_SET)!=0)//reset the cursor at the beginning of the file
    printf("error fseek SET\n");


  for (seq_nb=1; seq_nb<last_seq_nb; seq_nb++){
    init(buf, BUFLEN);
    if(feof(fsend)!=0)
      printf("EOF reached too early\n");
    if(ferror(fsend))
      printf("error reading file\n");

    printf("return of the fread : %d\n", fread(buf, sizeof(char), BUFLEN-1, fsend));

    if (sendto(s_data, buf, BUFLEN, 0, (struct sockaddr*)&SA_client, sizeof(SA_client))==-1)
      diep("sendto() data");
    printf("length of buf sent : %d\n", strlen(buf));
    //printf("buf sent : %s\n", buf);
    init(buf, BUFLEN);
    if (recvfrom(s_data, buf, BUFLEN, 0, (struct sockaddr*)&SA_client, &sLen)==-1)
      diep("recvfrom() ack");
    printf("I received ack : %s\n", buf);
  }
  //send last packet
  init(buf, BUFLEN);
  printf("return of the fread last packet : %d\n", fread(buf, sizeof(char), last_buf_size, fsend));
  if (sendto(s_data, buf, BUFLEN, 0, (struct sockaddr*)&SA_client, sizeof(SA_client))==-1)
    diep("sendto() data");
  printf("size of buf sent : %d\n", strlen(buf));
  init(buf,BUFLEN);
  if (recvfrom(s_data, buf, BUFLEN, 0, (struct sockaddr*)&SA_client, &sLen)==-1)
    diep("recvfrom() ack");
  printf("I received ack : %s\n", buf);


  fclose(fsend);
/***************************/

  close(s_con);
  close(s_data);
  return 0;
}
