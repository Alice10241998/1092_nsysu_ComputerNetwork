#include "header.h"

int client_threeway(void);
int request(void);

int client_sockfd, port;
string address;
struct sockaddr_in addr;
packet pkt_recv,pkt_send;
socklen_t addr_len;

int main(int argc,char* argv[])
{
	string IP;
	int send_port;
    
	if(argc == 1)
    {
		IP = "0.0.0.0";
		send_port = PORT;
	}
	else if(argc == 3)
    {
		IP = argv[1];
		send_port = atoi(argv[2]);
	}

    address = IP;
    port = send_port;
    if((client_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) <0)
    {
        cerr << "socket\n";
        exit(1);
    }
	memset(&addr, 0, sizeof(addr));  
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(address.c_str());
	addr.sin_port = htons(port);
    addr_len = sizeof(addr);
    
    memset(&pkt_send, 0, sizeof(packet));
    memset(&pkt_recv, 0, sizeof(packet));

	while(1)
		if(client_threeway())break;
	request();
}

int client_threeway()
{
    cout << "=====start the three-way handshake=====\n";
    srand(time(NULL)+5);
    pkt_send.seq = 1+rand()%10000;
    pkt_send.checkbit[0] = 1;
    sendto(client_sockfd, (void*)&pkt_send, sizeof(pkt_send), 0, (struct sockaddr*)&addr, addr_len);
    cout << "Send a packet(SYN) to " << address << " : " << port << endl;
    cout << "\tpacket(seq) : " << pkt_send.seq << endl;
    recvfrom(client_sockfd, (void*)&pkt_recv, sizeof(pkt_recv), 0, (struct sockaddr*)&addr, (socklen_t*)&addr_len);
    if(pkt_recv.checkbit[0] != 1 || pkt_recv.checkbit[1] != 1 )
    {
        cerr << "connect (three way)\n";
        return 0;
    }
    pkt_send.ack = pkt_recv.seq + 1 ;
    pkt_send.seq ++ ;
    cout << "Receive a packet(SYN/ACK) from " << address << " : " << port << endl;
    cout << "\tReceive a packet (seq_num = " << pkt_recv.seq << ",ack_num = " << pkt_recv.ack <<" )" << endl;
    sendto(client_sockfd, (void*)&pkt_send, sizeof(pkt_send), 0, (struct sockaddr *)&addr, addr_len);
    cout << "Send a packet to " << address << " : " << port << endl;
    cout << "=====complete the three-way handshake=====\n";
    return 1;
}

int request(void)
{
    while(1)
    {
        memset((void*)&pkt_send, 0, sizeof(pkt_send));
        cout << "input request (DNS/math/trans)" << endl;
        fgets(pkt_send.data, MSS, stdin);
        string s = pkt_send.data;
        pkt_send.ack = 0;
        pkt_send.request = 1;
        sendto(client_sockfd, (void*)&pkt_send, sizeof(pkt_send), 0, (struct sockaddr*)&addr, addr_len);
        memset((char*)&pkt_send, 0, sizeof(pkt_send));

        vector<string>cmd;
        while(1)
        {
            int pos = s.find(' ');
            if(pos == string::npos)
            {
                cmd.push_back(s);
                cmd.back().pop_back();
                break;
            }			
            cmd.push_back(s.substr(0,pos));
            s = s.substr(pos+1);
        }
        for(int i=0; i<cmd.size(); i=i+2)
        {
            memset((void*)&pkt_send, 0, sizeof(pkt_send));
            if(cmd[i]=="DNS"||cmd[i]=="math" )
            {
                recvfrom(client_sockfd, (void*)&pkt_recv, sizeof(pkt_recv), 0, (struct sockaddr*)&addr, (socklen_t*)&addr_len);
                cout << "\tReceive a packet (seq_num = " << pkt_recv.seq << " ),(" << "ack_num = "<< pkt_recv.ack << ")" << endl;
                pkt_send.seq = pkt_recv.ack ;
                pkt_send.ack = pkt_recv.seq + strlen(pkt_recv.data) + 1;
                pkt_send.request = 0;
                sendto(client_sockfd, (void*)&pkt_send, sizeof(pkt_send), 0, (struct sockaddr *)&addr, addr_len);
                cout << pkt_recv.data << endl;
            }
            else if(cmd[i] == "trans")
            {
                recvfrom(client_sockfd, (void*)&pkt_recv, sizeof(pkt_recv), 0, (struct sockaddr*)&addr, (socklen_t*)&addr_len);
                cout << "\tReceive a packet (seq_num = " << pkt_recv.seq << " ),(" << "ack_num = "<< pkt_recv.ack << ")" << endl;
                pkt_send.seq = pkt_recv.ack ;
                pkt_send.ack = pkt_recv.seq + strlen(pkt_recv.data) + 1;
                pkt_send.request = 0;
                sendto(client_sockfd, (void*)&pkt_send, sizeof(pkt_send), 0, (struct sockaddr*)&addr, addr_len);
                string filename = pkt_recv.data;
                cout << "Receiving file : " << filename << "...." << endl;
                FILE *f = fopen(("test/" + filename).c_str(),"wb");
                while(recvfrom(client_sockfd, (void*)&pkt_recv, sizeof(pkt_recv), 0, (struct sockaddr*)&addr, (socklen_t *)&addr_len))
                {
                    if(pkt_recv.fin == 1)break;
                    
                    pkt_send.datalen = 0;
                    pkt_send.seq = pkt_recv.ack ;
                    pkt_send.ack = pkt_recv.seq + pkt_recv.datalen;
                    pkt_send.request = 0;
                    cout << "\tReceive a packet (seq_num = " << pkt_recv.seq << " ),(" << "ack_num = "<< pkt_recv.ack << ")" << endl;
                    fwrite(pkt_recv.data,1,pkt_recv.datalen,f);
                    sendto(client_sockfd, (void*)&pkt_send, sizeof(pkt_send), 0, (struct sockaddr*)&addr, addr_len);

                }
                cout << "file accecpt : " << filename << endl <<endl;
                fclose(f);
            }
        }
    }
    return 1;
}
