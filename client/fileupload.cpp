#include "clientutil.h"

std::vector<string> stringdivide(string str,char colim)
{
	std::vector<string> vec;
	int i=0;
	string s="";
	while(i<str.length())
	{
		if(str[i]==colim)
		{
			vec.push_back(s);
			s="";
			i++;
		}
		else
		{
			s=s+str[i];
			i++;
		}
	}
	vec.push_back(s);
	return vec;
}
size_t calcFileSize(string filename,int test)
{
    //cout<<test<<endl;
	struct stat stat_obj;
	long long ans=stat(filename.c_str(),&stat_obj);
	if(ans==-1)
    {	
        return ans;
    }
	else 
    {
		return stat_obj.st_size;
    }
}
string uploadFile(string filename,int check) 
{
    //cout<<check<<endl;
    long long uploadfile_size = calcFileSize(filename,1);
    if (uploadfile_size == -1) 
    {
        cout << "file does not exist" << endl;
        return "$";
    }
    if (uploadfile_size == 0) 
    {
        cout << "no content in file to be uploaded" << endl;
        return "$";
    }
    size_t chunks = 524288;
    if (uploadfile_size < chunks) 
    {
        chunks = uploadfile_size;
    }
    int fd = open(filename.c_str(), O_RDONLY);
    if (fd == -1) 
    {
        perror("Error opening file");
        return "$";
    }
    char arr[chunks];
    string filehash = "";

    while (uploadfile_size > 0) 
    {
        ssize_t bytesRead = read(fd, arr, chunks);
        if (bytesRead <= 0) 
        {
            perror("Error reading file");
            close(fd);
            return "$";
        }

        string sha_chunk = calcsha(arr, bytesRead);
        filehash.append(sha_chunk.substr(0, 20));

        uploadfile_size -= bytesRead;

        if (uploadfile_size < chunks) 
        {
            chunks = uploadfile_size;
        }
    }
    close(fd);
    string final_hash = hashofhash(filehash);
    cout<<"piecehash:"<<final_hash<<"\n";
    return final_hash;
}
string calcsha(char *head,long long int chunks) 
{
	unsigned char md[20];
    unsigned char buf[40];
    SHA1((unsigned char *)head,chunks, md);
    for (int i = 0; i < 20; i++)
    {
        sprintf((char *)&(buf[i * 2]), "%02x", md[i]);
    }
    string md_1((char *)buf);
     cout<<md_1<<"\n";
    return md_1;

}
string hashofhash(string filehash) // stackoverflow
{
	
	string str_hash;
	unsigned char hashString[40];
	char *Buff;
	Buff = new char[filehash.length() + 1];
	strcpy(Buff, filehash.c_str());
	SHA1((unsigned char *)Buff, filehash.length(), hashString);
	int i;
	char fHashString[SHA_DIGEST_LENGTH * 2];
	for (i = 0; i < SHA_DIGEST_LENGTH; i++)
	{
		sprintf((char *)&(fHashString[i * 2]), "%02x", hashString[i]);
	}
	for (i = 0; i < SHA_DIGEST_LENGTH * 2; i++)
		str_hash += fHashString[i];
	return str_hash;
}

