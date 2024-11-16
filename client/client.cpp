
#include "fileupload.cpp"

long int chunk_size=524288; //512kb
string client_ip;
	std::string tracker1_ip="127.0.0.1";
	std::string tracker1_port="6050";
	std::string tracker2_ip="127.0.0.1";
	std::string tracker2_port="7050";
    //std::string col=":";

vector<string>command;

void *getMyServer(void *clientIP)
{
    // cout<<"Created getMyServer"<<"\n";
    auto int server_fd;
    struct sockaddr_in address;
    auto int addrlen = sizeof(address);
	auto int opt = 1;
	auto int new_socket;
	 string client_ip, SPORT;
    char *s_ip;
    string cip = *(string *)clientIP;
    vector<string> v = stringdivide(cip, ':');
    client_ip = v[0];
    SPORT = v[1];
    s_ip = new char[client_ip.length()];
    strcpy(s_ip, client_ip.c_str());
    //  Connection establishment code.
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(1);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR , &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(1);
    }
    address.sin_family = AF_INET;
    address.sin_port = htons(stoi(SPORT));

    if (inet_pton(AF_INET, s_ip, &address.sin_addr) <= 0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return clientIP;
    }

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(1);
    }
    if (listen(server_fd, 10) < 0)
    {
        perror("listen");
        exit(1);
    }
    	while(1)
    	{ 
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {            
            exit(1);
        }
       
	    int clientport_no=ntohs(address.sin_port);
		string clientport_str=to_string(clientport_no);
		
		int* argument=(int *)malloc(sizeof(*argument));
		*argument=new_socket;

    	int conn_sock=new_socket;
		char buffer[512]={0};
		int check=read(conn_sock,buffer,1024);
		if(check==0)
		return clientIP;
	
		string fname=string(buffer);
		cout<<fname<<"\n";
		size_t chunk=512; 
		long long filesize=calcFileSize(fname,1);
		cout<<filesize<<"\n";

		send(conn_sock,&filesize,sizeof(filesize),0);
		int fd = open(fname.c_str(), O_RDONLY);
			if (fd == -1) {
				perror("Error opening file");
				free(argument);
				close(conn_sock);
				break;
			}
			char buffer1[chunk];
			ssize_t bytesRead = 0;
			while ((bytesRead = read(fd, buffer1, chunk)) > 0 && filesize > 0) {
				ssize_t totalSent = 0;
				ssize_t bytesSent = 0;
				while (totalSent < bytesRead) {
					bytesSent = send(conn_sock, buffer1 + totalSent, bytesRead - totalSent, 0);
					if (bytesSent == -1) {
						perror("Error sending data");
						close(fd);
						free(argument);
						close(conn_sock);
						break;
					}
					totalSent += bytesSent;
				}
				memset(buffer1, '\0', chunk);
				filesize -= totalSent;

				if (filesize <= 0) {
					break;
				}
			}
			close(fd);
			free(argument);
			close(conn_sock);
		}
		return NULL;
	}

void p2pClient(string seed_port, string fname, string fneed,int t,bool test) 
{
	// cout<<t<<"\n";
	// cout<<test<<"\n";
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    if (sock < 0)
	 {
        perror("Socket creation error");
        return;
    }

    cout << seed_port << "\n";
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(stoi(seed_port));

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) 
	{
        perror("Invalid address/ Address not supported");
        close(sock);
        return;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
	{
        perror("Connection Failed");
        close(sock);
        return;
    }

    char reply_char_err[fneed.length() + 1];
    strcpy(reply_char_err, fneed.c_str());
    send(sock, reply_char_err, strlen(reply_char_err), 0);
    //cout << fname << "\n";

    int fd = open(fname.c_str(), O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	cout<<fd<<"\n";
    if (fd == -1) 
	{
        perror("Can't Open file");
        close(sock);
        return;
    }

    long long file_size = 0;
    if (recv(sock, &file_size, sizeof(file_size), 0) < 0) 
	{
        perror("Error while receiving file size");
        close(fd);
        close(sock);
        return;
    }

    cout << "File size in bytes: " << file_size << "\n";

    off_t offset = 0;
    while (file_size > 0) 
	{
        ssize_t bytesRead = recv(sock, buffer, sizeof(buffer), 0);
		cout<<bytesRead<<"\n";
        if (bytesRead <= 0) 
		{
            perror("Error while receiving data");
            break;
        }

        ssize_t bytesWritten = pwrite(fd, buffer, bytesRead, offset);
        if (bytesWritten < 0) 
		{
            perror("Error while writing to file");
            break;
        }

        file_size -= bytesRead;
        offset += bytesRead;

        if (file_size <= 0) 
		{
            break;
        }
    }
    cout << "Data transfer complete" << "\n";
    close(fd);
    close(sock);
	// cout<<t<<"\n";
	// cout<<test<<"\n";
}


int main(int argc, char* argv[])
{
	
	if(argc!=3)
	{
		cout<<"Invalid Arguments"<<"\n";
		exit(1);
	}
	string c_ip,c_port;


	client_ip=string(argv[1]);
	
	command=stringdivide(client_ip,':');
	c_ip=command[0],c_port=command[1];


		pthread_t serverthread;
		pthread_attr_t thread_attr2;
    int temp = pthread_attr_init(&thread_attr2);
    if (temp != 0) 
	{
        perror("Attribute creation failed");
        exit(1);
    }
   
    if( pthread_create( &serverthread , &thread_attr2 ,  getMyServer , (void*)&client_ip) < 0)
    {
        perror("could not create thread");
        return 1;
    }

	int sock_fd=socket(AF_INET,SOCK_STREAM,0);
	if(sock_fd<0)
	{
		cout<<"error in connection from client";
		exit(1);
	}
	struct sockaddr_in serv_addr;

	memset(&serv_addr, '0', sizeof(serv_addr));

	serv_addr.sin_family=AF_INET;
	serv_addr.sin_port=htons(stoi(tracker1_port));
	char* tracker1_ip_char=new char[tracker1_ip.length()+1];
	strcpy(tracker1_ip_char,tracker1_ip.c_str());

	if (inet_pton(AF_INET, tracker1_ip_char, &serv_addr.sin_addr) <= 0)
    {
        printf("\nInvalid address/ Address not supported \n");
        
        return -1;
    }

	int ret=connect(sock_fd,(struct sockaddr *)&serv_addr  , sizeof(serv_addr));
	if(ret<0)
	{
		cout<<"connection failed"<<"\n";
		exit(1);
	}

	
	auto int check=0;
	string opcode,username,password,uploadfile_name;
	while(1)
	{
		cout<<"Enter Command>> ";
		cin>>opcode;


        if(opcode=="create_user")
		{
			auto int validbit;
			cin>>username>>password;
			string col=":";
			string data=opcode+col+username+col+password;
			if(check==0)
			{
				char* data_char=new char[data.length()+1];
				strcpy(data_char,data.c_str());
				send(sock_fd,data_char,strlen(data_char),0);
				recv(sock_fd,&validbit,sizeof(validbit),0);
				if(validbit==1)
				{
					cout<<"User Created Successfully!! "<<"\n";
					
				}
				else
				{
					cout<<"You are already logged in!"<<"\n";
				}	
			}
			
			else
			{
				cout<<"Creation failed"<<"\n";
				
			}

		}
		else if(opcode=="login")
		{
			auto int validbit;
			cin>>username>>password;
			string col=":";
			string data=opcode+col+username+col+password;

			char* data_char=new char[data.length()+1];

			strcpy(data_char,data.c_str());
			
			send(sock_fd,data_char,strlen(data_char),0);

			recv(sock_fd,&validbit,sizeof(validbit),0);
			if(validbit==1)
			{
				check=1;
				cout<<"Logged in Successfully!!"<<"\n";
			}	
			else
			{
				cout<<"Check Username or Password.."<<"\n";
			}

		}
		else if(opcode=="upload_file")
		{
			if(check)
			{
				cin>>uploadfile_name;
				string grpid;
				cin>>grpid;
				char buffer[1024]={0};
				string col=":";
				string data=opcode+col+uploadfile_name;
				
				string upfile_hash=uploadFile(uploadfile_name,1);

				if(upfile_hash=="$")
					continue;
				long long fsize=calcFileSize(uploadfile_name,1);

				cout<<"file size is "<<fsize<<" Bytes"<<"\n";

				string request=opcode+col+upfile_hash+col+c_ip+col+uploadfile_name+col+c_port+col+grpid+col+to_string(fsize)+col+username;

				char *upfile_hash_char;

				upfile_hash_char=new char[request.length()+1];

				strcpy(upfile_hash_char,request.c_str());

				send(sock_fd,upfile_hash_char,strlen(upfile_hash_char),0);

				read(sock_fd,buffer,1024);

				cout<<string(buffer)<<"\n";
			}
			else
			{
				cout<<"Login first to upload.."<<"\n";
			}
		}
		else if(opcode=="download_file")
		{
			
			string grpid,file_name,dest;
			cin>>grpid>>file_name>>dest;
	
			if(check)
			{
				char buffer[4096]={0};
				string col=":";

				string request=opcode+col+file_name+col+grpid;

				char* request_char=new char[request.length()+1];

				strcpy(request_char,request.c_str());

				send(sock_fd,request_char,strlen(request_char),0);

				read(sock_fd,buffer,4096);

				cout<<string(buffer)<<"\n";

				std::vector<string> seeder_vec=stringdivide(string(buffer),'|');

				cout<<seeder_vec[(seeder_vec.size())-1]<<"\n";

				long long file_size=stoi(seeder_vec[(seeder_vec.size())-1]);

				int seederscount=seeder_vec.size()-1;

				seederinfo seederlist[seederscount];

				string testip,testport;

				for(int i=0;i<seeder_vec.size()-1;i++)
				{   
					cout<<seeder_vec.size()<<"\n";

					cout<<seeder_vec[0]<<"\n";
					string ipandport=seeder_vec[i];
					cout<<ipandport<<"\n";
					std::vector<string> temp=stringdivide(ipandport,':');

					seederinfo sinfo;

					sinfo.seeder_ip=temp[0];

					sinfo.seeder_port=temp[1];

					testip=temp[0];

					testport=temp[1];

					sinfo.filepath=file_name;

					sinfo.destpath=dest;

					sinfo.numofseeders=seederscount;

					seederlist[i]=sinfo;

				}
				//cout<<"hello15"<<"\n";

				p2pClient(testport,dest,file_name,1,true);

				//cout<<"hello16"<<"\n";

				string upfile_hash=uploadFile(dest,1);
				//cout<<"hello17"<<"\n";

				if(upfile_hash=="$")
					continue;
				// long long fsize=getfile_size(dest);
				//cout << "Before getfile_size" << "\n";
				long long fsize = calcFileSize(dest,1);
				cout << "After downloading filesize is: " << fsize << "\n";
				
			    request="upload_file"+col+upfile_hash+col+c_ip+col+dest+col+c_port+col+grpid+col+to_string(fsize)+col+username;

				char *upfile_hash_char;

				upfile_hash_char=new char[request.length()+1];

				strcpy(upfile_hash_char,request.c_str());

				send(sock_fd,upfile_hash_char,strlen(upfile_hash_char),0);

				read(sock_fd,buffer,1024);

				cout<<buffer<<"\n";

				request="add_download"+col+username+col+grpid+col+dest;

				char* request_down=new char[request.length()+1];

				strcpy(request_down,request.c_str());

				send(sock_fd,request_down,strlen(request_down),0);

			}
			else
			{
				cout<<"Login first to download.."<<"\n";
				
			}
		}
		else if(opcode=="create_group")
		{
			string grpid;
			cin>>grpid;
			if(check)
			{
			char buffer[1024]={0};

			string col=":";

			string temp1=opcode+col;
			string temp2=grpid+col+username;
			string request=temp1+temp2;

			char* request_char=new char[request.length()+1];

			strcpy(request_char,request.c_str());

			send(sock_fd,request_char,strlen(request_char),0);

			read(sock_fd,buffer,1024);

			cout<<string(buffer)<<"\n";
		}
		else
		{
				cout<<"Login first to create group.."<<"\n";
		}

		}
		else if(opcode=="list_groups")
		{
			if(check)
			{
			char buffer[1024]={0};
			//string col=":";
			string request=opcode;

			char* request_char=new char[request.length()+1];
			
			strcpy(request_char,request.c_str());

			send(sock_fd,request_char,strlen(request_char),0);

			read(sock_fd,buffer,1024);
			
			string grps=string(buffer);

			std::vector<string> grpnames;

			grpnames=stringdivide(grps,':');

			// for(int i=0;i<grpnames.size();i++)
			// 	cout<<grpnames[i]<<"\n";
			for(auto x:grpnames)
			{
				cout<<x<<"\n";
			}
		}
		else
		{
				cout<<"Login first.."<<"\n";
		}

		}
		else if(opcode=="join_group")
		{
			if(check==1)
			{
				char buffer[1024]={0};
				string grpid;
				cin>>grpid;
				string col=":";
				//string request=opcode+col+grpid+col+username;
				string temp1=opcode+col;
				string temp2=grpid+col+username;
				string request=temp1+temp2;
				char* request_char=new char[request.length()+1];

				strcpy(request_char,request.c_str());

				send(sock_fd,request_char,strlen(request_char),0);

				read(sock_fd,buffer,1024);

				cout<<string(buffer)<<"\n";

			}
			else
			{
				cout<<"Login first to join group.."<<"\n";
				
			}
			

		}
		else if(opcode=="leave_group")
		{
			if(check==1)
			{
				char buffer[1024]={0};
				string grpid;
				cin>>grpid;
				 string col=":";
				// string request=opcode+col+grpid+col+username;
					string temp1=opcode+col;
			string temp2=grpid+col+username;
			string request=temp1+temp2;
				char* request_char=new char[request.length()+1];

				strcpy(request_char,request.c_str());

				send(sock_fd,request_char,strlen(request_char),0);

				read(sock_fd,buffer,1024);

				cout<<string(buffer)<<"\n";

			}
			else
			{
				cout<<"Please login first"<<"\n";
				
			}

		}
		else if(opcode=="list_requests")
		{
			if(check==1)
			{
				char buffer[4096]={0};
				string grpid;
				cin>>grpid;
				 string col=":";
				// string request=opcode+col+grpid+col+username;
				string temp1=opcode+col;
			string temp2=grpid+col+username;
			string request=temp1+temp2;
				char* request_char=new char[request.length()+1];

				strcpy(request_char,request.c_str());

				send(sock_fd,request_char,strlen(request_char),0);

				read(sock_fd,buffer,1024);

				request=string(buffer);

				std::vector<string> join_req;

				join_req=stringdivide(request,':');

				// for(int itr=0;itr<join_req.size();itr++)
				// 	cout<<join_req[itr]<<"\n";
				for(auto x:join_req)
				{
					cout<<x<<"\n";
				}
			}
			else
			{
				cout<<"Login first.."<<"\n";
				
			}

		}
		else if(opcode=="accept_request")
		{
			if(check==1)
			{
				char buffer[1024]={0};
				string grpid,uid;
				cin>>grpid>>uid;
				string col=":";
				 //string request=opcode+col+grpid+col+uid+col+username;
					string temp1=opcode+col+grpid+col;
			string temp2=uid+col+username;
			 string request=temp1+temp2;
				char* request_char=new char[request.length()+1];

				strcpy(request_char,request.c_str());

				send(sock_fd,request_char,strlen(request_char),0);

				read(sock_fd,buffer,1024);

				cout<<string(buffer)<<"\n";
			}
			else
			{
				cout<<"Login first.."<<"\n";
				
			}	

		}
		else if(opcode=="list_files")
		{
			if(check==1)
			{
				char buffer[4096]={0};
				string grpid;
				cin>>grpid;
				 string col=":";
				// string request=opcode+col+grpid+col+username;
			string temp1=opcode+col;
			string temp2=grpid+col+username;
			string request=temp1+temp2;
				char* request_char=new char[request.length()+1];

				strcpy(request_char,request.c_str());

				send(sock_fd,request_char,strlen(request_char),0);

				read(sock_fd,buffer,1024);

				request=string(buffer);

				std::vector<string> files_list;

				files_list=stringdivide(request,':');

				for(int itr=0;itr<files_list.size();itr++)
				{
					cout<<files_list[itr]<<"->";
					stringstream tokenize(files_list[itr]);
					string file_name;
                    while(getline(tokenize , file_name, '/')) ;
					cout<<file_name<<"\n";
				}
			}
			else
			{
				cout<<"Login first.."<<"\n";
				
			}

		}
		else if(opcode=="logout")
		{
			if(check==1)
			{
				char buffer[4096]={0};
				string col=":";
				string request=opcode+col+username;
				char* request_char=new char[request.length()+1];

				strcpy(request_char,request.c_str());

				send(sock_fd,request_char,strlen(request_char),0);

				read(sock_fd,buffer,1024);

				cout<<string(buffer)<<"\n";

				check=0;

			}
			else
			{
				cout<<"You are'nt login in any network.."<<"\n";
			}
		}
		else if(opcode=="stop_share")
		{
			string grpid;
			string stop_share_file;
			cin>>grpid>>stop_share_file;
			if(check==1)
			{
				char buffer[4096]={0};
				string col=":";
				long long file_exist_check=calcFileSize(stop_share_file,1);
				if(file_exist_check==-1)
				{
					cout<<"file does not exist"<<"\n";
					continue;
				}
				string request=opcode+col+grpid+col+stop_share_file+col+username;
				char* request_char=new char[request.length()+1];
				strcpy(request_char,request.c_str());
				send(sock_fd,request_char,strlen(request_char),0);
				read(sock_fd,buffer,1024);
				cout<<string(buffer)<<"\n";

			}
			else
			{
				cout<<"you are not logged in! first login."<<"\n";
			}

		}
		else if(opcode=="show_downloads")
		{
				string col=":";
				char buffer[4096]={0};

				string request=opcode+col+username;

				char* request_char=new char[request.length()+1];

				strcpy(request_char,request.c_str());

				send(sock_fd,request_char,strlen(request_char),0);

				read(sock_fd,buffer,4096);

				request=string(buffer);

				std::vector<string> down_files;

				down_files=stringdivide(request,'|');

				for(int itr=0;itr<down_files.size();itr++) 
				{
					cout<<down_files[itr]<<"->";
					stringstream tokenize(down_files[itr]);
					string down_name;
                    while(getline(tokenize , down_name, '/')) ;
					cout<<down_name<<"\n";
				}
		}
	}
	return 0;
}






