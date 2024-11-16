#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include<dirent.h>
#include<sys/stat.h>
#include <openssl/sha.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream> //basic inp output
#include<string.h>
#include<sys/socket.h> //for socket creation
#include<errno.h>
#include<string>
#include <errno.h>
#include <sstream>
#include <stdio.h>
#include <map>
#include<set> 
#include <sys/socket.h> 
#include <cstring>
#include <cstdlib>
#include <fcntl.h>

using namespace std;

extern long int chunk_size;
string uploadFile(string filename,int);
string calcsha(char *,long long int);
size_t calcFileSize(string,int);
string hashofhash(string createhash);
vector<string>stringdivide(string,char);

struct seederinfo
{
	string seeder_ip;
	string seeder_port;
	string filepath;
	string destpath;
	int numofseeders;
	long long num_mychunks;
	long long start;
	
};





