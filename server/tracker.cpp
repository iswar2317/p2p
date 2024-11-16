#include "trackerutil.h"

	std::string tracker1_ip="127.0.0.1";
	std::string tracker1_port="6050";
	std::string tracker2_ip="127.0.0.1";
	std::string tracker2_port="7050";

pthread_t client_name;	

int main(int argc,char* argv[])
{
	int opt=1;
	
	if(argc !=3)
	{
		cout<<"Invalid arguments"<<endl;
		exit(1);
	}
	cout<<"TRACKER1 Info:"<<tracker1_ip<<":"<<tracker1_port<<"  "<<"TRACKER2 Info:"<<tracker2_ip<<":"<<tracker2_port<<endl;

	char* tracker1_ip_char=new char[tracker1_ip.length()+1];
	strcpy(tracker1_ip_char,tracker1_ip.c_str());

	int server_fd=socket(AF_INET,SOCK_STREAM,0);
	if(server_fd==0)
	{
		cout<<"error in connection";
		exit(1);
	}

	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR , &opt, sizeof(opt)))
    {
        cout<<"setsockopt"<<endl;
             exit(1);
    }

    int port=stoi(tracker1_port);
    struct sockaddr_in addr;
	addr.sin_family=AF_INET;
	addr.sin_port=htons(port);
	addr.sin_addr.s_addr=INADDR_ANY;
	int addrlen = sizeof(sockaddr);

	if (inet_pton(AF_INET, tracker1_ip_char, &addr.sin_addr) <= 0)
    {
        printf("\nInchk address given \n");

        return -1;
    }
     int res=bind(server_fd ,(struct sockaddr *)&addr , sizeof ( addr ) );

	if(res<0)
	{
          perror("bind");
		  exit(1);
	}	

	if(listen (server_fd, 34)==0)
	{
		cout<<"Server is in Listening state!"<<endl;
		
	}	
	else
	{    
		perror("listen");	
		exit(1);
	}

	int new_sock;

	while(1)
	{
		if((new_sock=accept( server_fd , (struct sockaddr *)&addr , (socklen_t*)&addrlen))<0)
		{
			cout<<"Error in establishing connection"<<endl;
			exit(1);
		}
		int client_port_num=ntohs(addr.sin_port);
		string client_port_str=to_string(client_port_num);
		int* argument=(int *)malloc(sizeof(*argument));
		*argument=new_sock;

		if(pthread_create(&client_name,0,client_handler,argument)<0)
		{
			perror("could not create thread");
			return 1;
		}
	}	

	return 0;
}
