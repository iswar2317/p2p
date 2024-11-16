nb## How to compile this:-
1. go to client directory
   * ```g++ client.cpp -o client  -pthread $(pkg-config --cflags --libs openssl)```
2. go to server directory
   * ```g++ tracker.cpp -o tracker -pthread $(pkg-config --cflags --libs openssl)```

**To run tracker** :-  ./tracker trackerinfo.txt 1  
**To run client** :-  ./client 127.0.0.1:8050 trackerinfo.txt

### CLIENT COMMANDS 

- Create User Account: ``` create_user user_name password``` 
- Login: ``` login user_name password``` 
- Create Group: ``` create_group groupid``` 
- Join Group: ``` join_group groupid``` 
- List pending join: ``` list_requests groupid``` 
- Accept Group Joining Request: ``` accept_request groupid user_name	``` 
- List All Group In Network: ``` list_groups``` 
- Upload Files:
  - ``` upload_file video.mp4 groupid``` 
  - ``` upload_file image.jpg groupid``` 
  - ``` upload_file document.pdf groupid	``` 
  - ``` upload_file song.mp4 groupid``` 
- List shareable files:``` list_files groupid``` 
- Show ongoing downloads: ``` show_downloads``` 
- Stop sharing file: ``` stop_share groupid document.pdf``` 
- Logout: ```logout```
- Leave group: ```leave_group groupid```

### About

- Tracker: The tracker is a special server that keeps track of the connected peers. Maintains information of clients with their files(shared by client) to assist the clients for the communication between peers.
- Peers/Client: Users downloading from a BitTorrent swarm are commonly referred to as “leechers” or “peers. 
- For downloading, one peer – who has a complete copy of all the files in the torrent – joins the network so other users can download the data.
- All data transfers are encryped and verified using SHA.
- Reference link for SHA hash being used:-https://stackoverflow.com/questions/2262386/generate-sha256-with-openssl-and-c/10632725

**Note** :- Since I am using Macbook and have done this enitely on this so some minor hardcodes are there like if tracker is closed with a paticular port then we need to upadate it again in code as of now in both client and tracker side since in Mac bind address failed is coming and manually we need to delete the proccess with its pid to reuse the port,I have tried setsockopt too yet it did'nt work.