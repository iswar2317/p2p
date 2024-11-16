#include<pthread.h>
#include<stdlib.h>
#include<dirent.h>
#include<sys/stat.h>
#include <openssl/sha.h>
#include <netinet/in.h>
#include<sstream>
#include<iostream> 
#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h> 
#include<sys/types.h> 
#include<errno.h>
#include<unistd.h>
#include <map>
#include<set>
#include <cstdlib>
#include <cstring>
#include <sys/socket.h> 
#include <string>
using namespace std;


struct file_data
{
	string userid;
	string hash;
	string ip;
	string port;
	long int file_size;
	string grpid;
	bool isactive;	
};
struct grp_file_data{
	string userid;
	string fname;
	string ip;
	string port;
	bool isactive;
};
struct group
{
	string admin;
};

vector<string>stringdivide(string str,char delim)
{
	std::vector<string> v;
	int i=0;
	string s="";
	while(i<str.length())
	{
		if(str[i]==delim)
		{
			v.push_back(s);
			s="";
			i++;
		}
		else
		{
			s=s+str[i];
			i++;
		}
	}
	v.push_back(s);
	return v;
}





std::vector<string> cmd;
map<string,vector<string> >admin_req;//map for admin to pending requests
map<string,group>grp_map;//map group id to group;
map<string,vector<string> >grp_users;//map grp id to users
map<string,vector<grp_file_data> >grp_file_map;//group_name map to file data
map<string,vector<pair<string,string> > >user_files;//map userid to to pair of grpid and filename
std::map<string, vector<file_data> > file_map;// map filename to filedata
set<string>file_set;//set contains filenames present in group
map<string,string>user_pass;
map<string,vector<pair<string,string> > >downloads;
void *client_handler(void* sock_num)
{
		int conn_sock=*((int *)sock_num);
		
		while(1)
		{	
			char buffer[1024]={0};
			int status=read(conn_sock,buffer,1024);
			if(status==0)
				return sock_num;

			
			string buffer_str=string(buffer);

			cmd=stringdivide(buffer_str,':');

			struct file_data fileinfo;
			struct grp_file_data groupinfo;

			if(cmd[0]=="login")
			{

				auto int chk=0;
				if(user_pass[cmd[1]]==cmd[2])
				{
					
					chk=1;
					
				}
				if(chk==1)
				{
					string uid=cmd[1];
					vector<pair<string,string> > v=user_files[uid];
				if(v.size()!=0)
				{
					
					for(auto j=v.begin();j!=v.end();j++)
					{
						pair<string,string>p=*j;

						string logout_file=p.first;

						string logout_grp=p.second;

						std::vector<grp_file_data> logout_grp_vec=grp_file_map[logout_grp];

						for(auto k=logout_grp_vec.begin();k!=logout_grp_vec.end();k++)
						{
							struct grp_file_data gfd=*k;
							if(gfd.userid==uid)
							{
								cout<<gfd.fname<<endl;
								gfd.isactive=true;
							}
							*k=gfd;
						}
						grp_file_map[logout_grp]=logout_grp_vec;

						std::vector<file_data> logout_file_vec=file_map[logout_file];

						for(auto x=logout_file_vec.begin();x!=logout_file_vec.end();x++)

						{
							struct file_data fd=*x;
							if(fd.userid==uid)
								fd.isactive=true;
							 *x=fd;

						}

						file_map[logout_file]=logout_file_vec;

					}
				}
			}

				send(conn_sock,&chk,sizeof(chk),0);
			
		}


			else if(cmd[0]=="upload_file")
			{
				string reply;
				
				int flag=0;
				if(grp_map.find(cmd[5])==grp_map.end())
				{
					reply="group does not exist";
					char* reply_char_err=new char[reply.length()+1];
					strcpy(reply_char_err,reply.c_str());
					send(conn_sock,reply_char_err,strlen(reply_char_err),0);
					continue;
				}

				vector<string> gusers=grp_users[cmd[5]];
				int gflag=0;
				for(auto it=gusers.begin();it!=gusers.end();it++)
				{
					string ga=*it;
					if(ga==cmd[7])
					{
						gflag=1;
						break;
					}

				}
				if(gflag==0)
				{
					reply="You are not a member of this group";
					char* reply_char_err=new char[reply.length()+1];
					strcpy(reply_char_err,reply.c_str());
					send(conn_sock,reply_char_err,strlen(reply_char_err),0);
					continue;

				}
				string fname=cmd[3];
				fileinfo.ip=cmd[2];
				fileinfo.hash=cmd[1];
				fileinfo.port=cmd[4];
				fileinfo.grpid=cmd[5];
				fileinfo.userid=cmd[7];
				stringstream so(cmd[6]);
				long int x=0;
				so>>x;
				fileinfo.file_size=x;
				fileinfo.isactive=true;


				groupinfo.fname=cmd[3];
				groupinfo.ip=cmd[2];
				groupinfo.port=cmd[4];
				groupinfo.userid=cmd[7];
				groupinfo.isactive=true;

			    vector<file_data> v=file_map[fname];
				pair<string,string>p;
				p.first=fname;
				p.second=cmd[5];
				
				if(v.size()==0)
				{
					file_map[fname].push_back(fileinfo);
					grp_file_map[cmd[5]].push_back(groupinfo);
					user_files[cmd[7]].push_back(p);
					reply="File is Successfully Shared!";
				}
				else
				{
				for (auto j=v.begin();j!=v.end();j++)
				{
					struct file_data f=*j;
					if(f.userid==fileinfo.userid && f.grpid==fileinfo.grpid)
					{
						flag=1;
						break;
					}
				}
				if(flag)
				{
					reply="Already a shared File!";			
				}
				else
				{
					
					file_map[fname].push_back(fileinfo);

					grp_file_map[cmd[5]].push_back(groupinfo);

					user_files[cmd[7]].push_back(p);

					reply="File is Successfully Shared!";

				}
			}
				char* reply_char=new char[reply.length()+1];

				strcpy(reply_char,reply.c_str());

				send(conn_sock,reply_char,strlen(reply_char),0);

			}
		else if(cmd[0]=="create_user")
			{
				int chk=0;

				user_pass[cmd[1]]=cmd[2];
				cout<<cmd[1]<<":"<<cmd[2]<<endl;
				chk=1;

				send(conn_sock,&chk,sizeof(chk),0);
			}
			else if(cmd[0]=="create_group")
			{

				string grp_id=cmd[1];

				group g;

				g.admin=cmd[2];

				grp_map[grp_id]=g;

				cout<<grp_map[grp_id].admin<<" is admin"<<endl;

				string reply;
				vector<string> req;
				if(grp_map.find(grp_id)!=grp_map.end())
				{
					admin_req[g.admin]=req;
					grp_users[grp_id].push_back(cmd[2]);
					reply="Group was created successfully!!";
				}
				else
				{
					reply="Group can't be created..";
				}
				char* reply_char=new char[reply.length()+1];
				strcpy(reply_char,reply.c_str());
				send(conn_sock,reply_char,strlen(reply_char),0);
			}
			else if(cmd[0]=="list_groups")
			{
				string grps="";
				string del=":";
				if(grp_map.empty())
				{
					string reply="No groups are present";
					char* reply_char_err=new char[reply.length()+1];
					strcpy(reply_char_err,reply.c_str());
					send(conn_sock,reply_char_err,strlen(reply_char_err),0);
					continue;
				}
				for(auto i:grp_map)
				{
					cout<<i.first<<endl;
					grps=grps+i.first+del;
				}
				grps.pop_back();
				char* grps_char=new char[grps.length()+1];
				strcpy(grps_char,grps.c_str());
				send(conn_sock,grps_char,strlen(grps_char),0);
			}
			else if(cmd[0]=="join_group")
			{
				string reply;
				if(grp_map.find(cmd[1])==grp_map.end())
				{
					reply="Group doesn't exist";
					char* reply_char_err=new char[reply.length()+1];
					strcpy(reply_char_err,reply.c_str());
					send(conn_sock,reply_char_err,strlen(reply_char_err),0);
					continue;
				}
				else
				{
					admin_req[grp_map[cmd[1]].admin].push_back(cmd[2]);
					reply="Join request is send to admin";
					char* reply_char_err=new char[reply.length()+1];
					strcpy(reply_char_err,reply.c_str());
					send(conn_sock,reply_char_err,strlen(reply_char_err),0);					

				}

			}
			else if(cmd[0]=="leave_group")
			{
				string reply;
				if(grp_map.find(cmd[1])==grp_map.end())
				{
					reply="Group does'nt exist";
					char* reply_char_err=new char[reply.length()+1];
					strcpy(reply_char_err,reply.c_str());
					send(conn_sock,reply_char_err,strlen(reply_char_err),0);
					continue;
				}
				else
				{
					int user_ex_fl=0;
					vector<string>:: iterator it1;
					std::vector<string> v=grp_users[cmd[1]];
					int i=0;
					for(i=0;i<v.size();i++)
					{
						cout<<v[i]<<" ";
						if(v[i]==cmd[2])
						{
							user_ex_fl=1;
							break;
						}
					}
					cout<<i;
					cout<<endl;
					if(user_ex_fl==0)
						reply="You are not a member of this group";
					else
					{
						grp_users[cmd[1]].erase(grp_users[cmd[1]].begin()+i);
						reply="You have left group "+cmd[1];
					}
					char* reply_char_err=new char[reply.length()+1];
					strcpy(reply_char_err,reply.c_str());
					send(conn_sock,reply_char_err,strlen(reply_char_err),0);

				}

			}
			else if(cmd[0]=="list_requests")
			{
				string ad=cmd[2];
				string grp=cmd[1];
				string reply="";
				string del=":";
				cout<<"admin:- "<<grp_map[grp].admin<<endl;
				cout<<"req:- "<<ad<<endl;
				if(grp_map[grp].admin!=ad)
				{
					reply="You are not a admin of this group";
					char* reply_char_err=new char[reply.length()+1];
					strcpy(reply_char_err,reply.c_str());
					send(conn_sock,reply_char_err,strlen(reply_char_err),0);

				}
				else
				{
					std::vector<string> temp_v=admin_req[ad];
					if(temp_v.size()==0)
						reply="No pending requests";
					else
					{
						for(int i=0;i<temp_v.size();i++)
						{
							reply=reply+temp_v[i]+del;
						}
						reply.pop_back();
					}
					char* reply_char_err=new char[reply.length()+1];
					strcpy(reply_char_err,reply.c_str());
					send(conn_sock,reply_char_err,strlen(reply_char_err),0);
				}
			}
			else if(cmd[0]=="accept_request")
			{
				string grp=cmd[1];
				string uid=cmd[2];
				string ad=cmd[3];
				string reply="";
				if(grp_map[grp].admin!=ad)
				{
					reply="You are not a admin of this group";
					char* reply_char_err=new char[reply.length()+1];
					strcpy(reply_char_err,reply.c_str());
					send(conn_sock,reply_char_err,strlen(reply_char_err),0);

				}
				else
				{
					int user_ex_fl=0;
					grp_users[grp].push_back(uid);
					std::vector<string> v=admin_req[ad];
					int i=0;
					for(i=0;i<v.size();i++)
					{
						//cout<<v[i]<<" ";
						if(v[i]==uid)
						{
							user_ex_fl=1;
							break;
						}
					}
					cout<<i;
					cout<<endl;
					if(user_ex_fl==0)
						reply="User is not there in join_request list";
					else
					{
						admin_req[ad].erase(admin_req[ad].begin()+i);
						reply=uid+" is now a member of "+grp;
					}
					char* reply_char_err=new char[reply.length()+1];
					strcpy(reply_char_err,reply.c_str());
					send(conn_sock,reply_char_err,strlen(reply_char_err),0);

				}

			}
			else if(cmd[0]=="list_files")
			{
				string grp=cmd[1];
				string reply="";
				string del=":";
				std::vector<grp_file_data> v= grp_file_map[grp];
				file_set.clear();
				if(v.size()==0)
				{
					reply="NO files are present in group to share";
				}
				else
				{
					for (auto j=v.begin();j!=v.end();j++)
					{
						//cout<<"size"<<v.size()<<endl;
						struct grp_file_data g=*j;
						if(g.isactive==true)
						{

							file_set.insert(g.fname);
						}
					}
					set<string>::iterator itr;
					for(itr=file_set.begin();itr!=file_set.end();++itr)
					{
						reply=reply+(*itr)+del;
					}
					reply.pop_back();
		        }	
		        	char* reply_char_err=new char[reply.length()+1];
					strcpy(reply_char_err,reply.c_str());
					send(conn_sock,reply_char_err,strlen(reply_char_err),0);

			}

			else if(cmd[0]=="stop_share")
			{
				string grp=cmd[1];
				string stop_file=cmd[2];
				string uid=cmd[3];
				string reply="";
				int flag=0;
				std::vector<pair<string,string> > v=user_files[uid];
				if(v.size()!=0)
				{
					for(int j=0;j<v.size();j++)
					{
						pair<string,string>p=v[j];
						if(p.first==stop_file && p.second==grp)
						{
							flag=1;
							user_files[uid].erase(user_files[uid].begin()+j);
							break;
						}
					}
					if(flag==0)
					{
						reply="you have not shared this file in this group";
						char* reply_char_err=new char[reply.length()+1];
						strcpy(reply_char_err,reply.c_str());
						send(conn_sock,reply_char_err,strlen(reply_char_err),0);
						continue;
					}

				
				std::vector<struct grp_file_data> v1=grp_file_map[grp];
				for(int j=0;j<v1.size();j++)
				{
					struct grp_file_data g=v1[j];
					if(g.userid==uid && g.fname==stop_file)
					{
						grp_file_map[grp].erase(grp_file_map[grp].begin()+j);
						break;
					}
				}
				std::vector<struct file_data> v2=file_map[stop_file];
				for(int j=0;j<v1.size();j++)
				{
					struct file_data f=v2[j];
					if(f.userid==uid && f.grpid==grp)
					{
						file_map[stop_file].erase(file_map[stop_file].begin()+j);
						break;
					}
				}
				reply="file is now not sharable in "+grp;
				
			}
			else{
				reply="You have not shared any file yet";
			}
				char* reply_char_err=new char[reply.length()+1];
				strcpy(reply_char_err,reply.c_str());
				send(conn_sock,reply_char_err,strlen(reply_char_err),0);	

			}
			else if(cmd[0]=="download_file")
			{
				string grp=cmd[2];
				string down_file=cmd[1];
				string reply="";
				string del=":";
				vector<struct file_data> v=file_map[down_file];
				if(v.size()!=0)
				{
					string fsize="";
					for(auto j=v.begin();j!=v.end();j++)
					{
						struct file_data f=*j;
						fsize=to_string(f.file_size);
						if(f.grpid==grp && f.isactive==true)
						{
							reply=reply+f.ip+del+f.port+"|";
						}
					}
					if(reply.length()>0)
						reply=reply+fsize;
				}
				else
					reply="File is not there in group";

				char* reply_char_err=new char[reply.length()+1];
				strcpy(reply_char_err,reply.c_str());
				send(conn_sock,reply_char_err,strlen(reply_char_err),0);


			}
		
			else if(cmd[0]=="show_downloads")
			{
				string reply="";
				std::vector<pair<string,string> > v=downloads[cmd[1]];
				for(auto j=v.begin();j!=v.end();j++)
				{
					pair<string,string>p=*j;
					reply=reply+p.first+":"+p.second+"|";
				}
				reply.pop_back();
				char* reply_char_err=new char[reply.length()+1];
				strcpy(reply_char_err,reply.c_str());
				send(conn_sock,reply_char_err,strlen(reply_char_err),0);
			}
			else if(cmd[0]=="add_download")
			{
				pair<string,string>p;
				p.first=cmd[2];
				p.second=cmd[3];
				downloads[cmd[1]].push_back(p);
			}
				else if(cmd[0]=="logout")
			{
				string uid=cmd[1];
				string reply="";
				std::vector<pair<string,string> > v=user_files[uid];
				if(v.size()!=0)
				{
					reply="Loggedout successfully!!";
					for(auto j=v.begin();j!=v.end();j++)
					{
						pair<string,string>p=*j;
						string logout_file=p.first;
						string logout_grp=p.second;
						std::vector<grp_file_data> logout_grp_vec=grp_file_map[logout_grp];
						for(auto k=logout_grp_vec.begin();k!=logout_grp_vec.end();k++)
						{
							struct grp_file_data gfd=*k;
							if(gfd.userid==uid)
							{
								cout<<gfd.fname<<endl;
								gfd.isactive=false;
							}
							*k=gfd;
						}
						grp_file_map[logout_grp]=logout_grp_vec;
						std::vector<file_data> logout_file_vec=file_map[logout_file];
						for(auto x=logout_file_vec.begin();x!=logout_file_vec.end();x++)
						{
							struct file_data fd=*x;
							if(fd.userid==uid)
								fd.isactive=false;
							*x=fd;

						}
						file_map[logout_file]=logout_file_vec;
					}
				}
				else
				{
					
					reply="logout successfully";
				}
				char* reply_char_err=new char[reply.length()+1];
				strcpy(reply_char_err,reply.c_str());
				send(conn_sock,reply_char_err,strlen(reply_char_err),0);
			}
		}
	return sock_num;
}


