#include <stdlib.h>
#include <stdio.h>

void diep(char *s)
{
    perror(s);
    exit(1);
}

void init(char* s, int len){
	int i;
	for(i=0; i<len; i++){
		s[i]=0;
	}
}

void display(char* s, int len){
	int i;
	for(i=0; i<len; i++){
		printf("%c", s[i]);
	}
	printf("\n");
}

int get_port(char* s, int len){
	if(len<4){
		printf("Error in getting the nb of port. By default, the port number has 4 digits. The string has less than 4 characters.\n");
		return 0;
	}
	else{
		int res;
		char temp[4];
		int i;
		for(i=0; i<4; i++){
			temp[i]=s[i+len-4];
		}
  	printf("in the get_port : %s\n",temp);
	res = atoi(temp);
  	printf("in the get_port : %d\n", res);
	return res;
	}
}

/*void sendable_seq_nb(char* s, int len){ // à terminer : le but est ici de rendre cette chaine de caractère de type 1 en 000001... OKLM
  int a = strlen(s);
  char temp[a] = {0};
  strcat(temp, s);
  printf("in sendab... : %s\n", temp);
  int i;
  for(i=0; i<len-strlen(temp); i++){
    s[i]=0;
  }
  for(i; i<len; i++){
    s[i]=temp[i-strlen(temp)];
  }
}*/

int get_file_extension_size(char* s, int len){
	int length =0;
        int i = 0;
	while(s[i]!='.'){
        	//do nothing
        	i++;
	}
	while(s[i]!=0){
		length++;
		i++;
	}
	return length;
}

void get_file_extension(char* s, int len, char* res, int lenR){
	int i;
	for(i=0; i<lenR; i++){
		res[i]=s[i + len-lenR];
	}
}


