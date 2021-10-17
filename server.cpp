#include "header.h"

void server_threeway();
void choice();
void DNS(string);
void math(string);
void trans(string);
int detect_loss();

int server_port, server_sockfd;
string server_ip;
struct sockaddr_in server_addr, client_addr;
packet pkt_send, pkt_recv;
socklen_t addr_len;

void para()
{
	cout << "=====Parameter=====" << endl;
	cout << "RTT : " << RTT << " ms" << endl;
	cout << "threshold : " << THRESHOLD << " bytes" << endl;
	cout << "MSS : " << MSS << " bytes" << endl;
	cout << "buffer size : " << BUFFER_SIZE << " bytes" << endl;
	cout << "Server's IP is " << server_ip << endl;
	cout << "Server is listening on port " << ntohs(server_addr.sin_port) << endl;
	cout << "===============" << endl;
}

int main(int argc,char* argv[])
{
	srand(time(NULL));

	server_port = atoi(argv[1]); //字串轉數字
	server_ip = LOCAL_IP;

	para();

    server_sockfd = socket(AF_INET,SOCK_DGRAM,0);
	if (server_sockfd < 0)
	{
		cerr << "socket\n";
		exit(1);
	}

	memset(&server_addr, 0,sizeof(struct sockaddr_in));
	memset(&client_addr, 0,sizeof(struct sockaddr_in));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.s_addr = htons(INADDR_ANY);
	addr_len = sizeof(client_addr);

	if(bind(server_sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)))
	{
		cerr << "server_sockfd bind failed" <<endl;
		exit(1);
	}

	cout << "Listening for client..." << endl;
	while(1)
	{	
	    recvfrom(server_sockfd, (void*)&pkt_recv, sizeof(pkt_recv), 0, (struct sockaddr*)&client_addr, (socklen_t*)&addr_len);
		if(fork() == 0) break;
	}

    server_sockfd = socket(AF_INET,SOCK_DGRAM,0);
	server_addr.sin_port = htons(0);
	bind(server_sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
	connect(server_sockfd ,(struct sockaddr*)&client_addr, sizeof(server_addr));

	server_threeway();
    choice();
	return 0;
}

void server_threeway()
{
	cout << "=====Start the three-way handshake=====" << endl;
    cout << "\tReceive a packet (seq_num = " << pkt_recv.seq << " , " << "ack_num = "<< pkt_recv.ack << ")" << endl;
	pkt_send = pkt_recv;
	pkt_send.checkbit[1] = 1;
	pkt_send.seq = 1 + rand() % 10000;
	pkt_send.ack = pkt_recv.seq + 1;
	sendto(server_sockfd, (void*)&pkt_send, sizeof(pkt_send), 0, (struct sockaddr*)&client_addr, addr_len);
	cout << "\tSent a packet (seq_num = " << pkt_send.seq << " ," << "ack_num = "<< pkt_send.ack << ")" << endl;

	recvfrom(server_sockfd, (void*)&pkt_recv, sizeof(pkt_recv), 0, (struct sockaddr*)&client_addr, (socklen_t*)&addr_len);
	cout << "\tReceive a packet (seq_num = " << pkt_recv.seq << " , " << "ack_num = "<< pkt_recv.ack << ")" << endl;
	cout << "=====Complete the three-way handshake=====" << endl;
}

void choice()
{
	while(1)
    {
		recvfrom(server_sockfd, (void*)&pkt_recv, sizeof(pkt_recv), 0, (struct sockaddr *)&client_addr, (socklen_t *)&addr_len);
        pkt_send.seq = pkt_recv.ack;
		pkt_send.ack = pkt_recv.seq + strlen(pkt_recv.data) + 1;
		cout << "\tReceive a packet (seq_num = " << pkt_recv.seq << " , " << "ack_num = "<< pkt_recv.ack << ")" << endl;
		
		vector<string>cmd;
		string s = pkt_recv.data;
		while(1)
        {
			int pos = s.find(' ');
			if( pos == string::npos)
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
			if( cmd[i] == "DNS" )
				DNS(cmd[i+1]);	
			else if (cmd[i] == "math")
				math(cmd[i+1]);
			else if (cmd[i] == "trans")
				trans(cmd[i+1]);
			}
    }
}

void DNS(string cmd)
{
	cout << "======DNS====="<<endl;
	string s = "";
	stringstream ss;
	char ip[INET6_ADDRSTRLEN];
	struct addrinfo result,*res,*p;

	memset((void*)&pkt_send, '\0', sizeof(pkt_send));
	memset(&result, 0, sizeof(result));
	result.ai_family = AF_UNSPEC;
	result.ai_socktype = SOCK_STREAM;
	
	if(getaddrinfo(cmd.c_str(), NULL, &result, &res) != 0 )
    {
		cerr << "dns error.\n";
		exit(0);
	}

	s += "IP addresses for " + cmd + ":\n";
	for(p=res; p!=NULL; p=p->ai_next)
    {
		void *info;
		string ipver;
		if(p->ai_family == AF_INET)
        {
			struct sockaddr_in *ipv4 = (struct sockaddr_in*)p->ai_addr;
			info = &(ipv4->sin_addr);
			ipver = "IPv4";
		}
		else
        {
			struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
			info = &(ipv6->sin6_addr);
			ipver = "IPv6";
		}
		inet_ntop(p->ai_family, info, ip, sizeof(ip));
		s += ipver + " : " + ip + '\n';
	}

	freeaddrinfo(res);
	pkt_send.seq = pkt_recv.ack ;
	pkt_send.ack = pkt_recv.seq + strlen(pkt_recv.data) ;
	strcpy(pkt_send.data, s.c_str());
	sendto(server_sockfd, (void*)&pkt_send, sizeof(pkt_send), 0, (struct sockaddr*)&client_addr, addr_len);
	recvfrom(server_sockfd, (void*)&pkt_recv, sizeof(pkt_recv), 0, (struct sockaddr*)&client_addr, (socklen_t *)&addr_len);
	pkt_send.seq = pkt_recv.ack ;
	pkt_send.ack = pkt_recv.seq + strlen(pkt_recv.data) + 1;
	cout << "\tReceive a packet (seq_num = " << pkt_recv.seq << " , " << "ack_num = "<< pkt_recv.ack << ")" << endl;
	cout << "=====Finish=====" << endl <<endl;
}

void math(string s)
{
	cout << "=====math====="<<endl;

	float n1,n2,ans;
	stringstream ss;
	string op;

	memset((void*)&pkt_send, '\0', sizeof(pkt_send));

	op = s.substr(0, s.find(','));
	s  = s.substr(s.find(',') + 1 );
	n1 = atof(s.substr(0, s.find(',')).c_str());
	s  = s.substr(s.find(',') + 1 );
	n2 = atof(s.c_str());

	if(op == "add")
	{
		ans = n1 + n2 ;
		op = "+";
	}
	else if(op == "sub")
	{
		ans = n1 - n2 ;
		op = "-";
	}
	else if(op == "mul")
	{
		ans = n1 * n2 ;
		op = "*";
	}
	else if(op == "div")
	{
		ans = n1 / n2 ;
		op = "/";
	}
	else if(op == "pow")
	{
		ans = pow(n1,n2);
		op = "^";
	}
	else if(op == "sqr")
	{
		ans = pow(n1, 1/n2);
	}

	s.clear();
	ss << n1;
	ss << ' ';
	ss << op;
	ss << ' ';
	ss << n2;
	ss << " = ";
	ss << ans;
	ss << endl;

	pkt_send.seq = pkt_recv.ack ;
	pkt_send.ack = pkt_recv.seq + strlen(pkt_recv.data) + 1 ;
	strcpy(pkt_send.data,ss.str().c_str());
	sendto(server_sockfd,(void*)&pkt_send, sizeof(pkt_send), 0, (struct sockaddr*)&client_addr, addr_len);
	recvfrom(server_sockfd, (void*)&pkt_recv, sizeof(pkt_recv), 0, (struct sockaddr*)&client_addr, (socklen_t*)&addr_len);
	pkt_send.seq = pkt_recv.ack;
	pkt_send.ack = pkt_recv.seq + strlen(pkt_recv.data);
	cout << "\tReceive a packet (seq_num = " << pkt_recv.seq << " , " << "ack_num = "<< pkt_recv.ack << ")" << endl;
	cout << "=====Finish=====" << endl <<endl;
}

void trans (string name)
{
	cout << "=====transmission file=====" << endl;

	char buf[MSS];
	int n;

	memset((void*)&pkt_send, '\0', sizeof(pkt_send));
	strcpy(pkt_send.data, name.c_str());
	pkt_send.seq = pkt_recv.ack ;
	pkt_send.ack = pkt_recv.seq + 1 + strlen(pkt_recv.data);
	
	cout << "\tsend a packet at : " << pkt_send.seq << " bytes\n";
	
	sendto(server_sockfd, (void*)&pkt_send, sizeof(pkt_send), 0, (struct sockaddr*)&client_addr, addr_len);
	recvfrom(server_sockfd, (void*)&pkt_recv, sizeof(pkt_recv), 0, (struct sockaddr*)&client_addr, (socklen_t*)&addr_len);
	cout << "\tReceive a packet (seq_num = " << pkt_recv.seq << " , " << "ack_num = "<< pkt_recv.ack << ")" << endl;
	
	memset(buf, '\0', sizeof(buf));
	memset((void*)&pkt_send, '\0', sizeof(pkt_send));

	FILE *f = fopen(name.c_str(), "rb");
	while((n = fread(pkt_send.data, 1, sizeof(pkt_send.data), f)) > 0)
	{
		memset(buf, '\0', sizeof(pkt_send.data));
		pkt_send.datalen = n;
		pkt_send.seq = pkt_recv.ack ;
		pkt_send.ack = pkt_recv.seq + 1 + pkt_recv.datalen;

		int loss_flag = detect_loss();

		if(loss_flag == 1)
			cout << "detect loss at " << pkt_send.seq << "byte" << endl;
		else
			cout << "\tsend a packet at : " << pkt_send.seq << " bytes\n";
		
		sendto(server_sockfd, (void*)&pkt_send, sizeof(pkt_send), 0, (struct sockaddr *)&client_addr, addr_len);
		recvfrom(server_sockfd, (void*)&pkt_recv, sizeof(pkt_recv), 0, (struct sockaddr *)&client_addr, (socklen_t *)&addr_len);
		
		if(loss_flag != 1)
		cout << "\tReceive a packet (seq_num = " << pkt_recv.seq << " , " << "ack_num = "<< pkt_recv.ack << ")" << endl;
		
		memset(buf, '\0', sizeof(buf));
		memset((void*)&pkt_send, '\0', sizeof(pkt_send));
		memset(pkt_send.data, '\0', sizeof(pkt_send.data));
	}

	fclose(f);

	memset(buf, '\0', sizeof(buf));
	memset((void*)&pkt_send, '\0', sizeof(pkt_send));

	pkt_send.fin = 1;
	pkt_send.seq = pkt_recv.ack ;
	pkt_send.ack = pkt_recv.seq + 1 + strlen(pkt_recv.data);
	
	sendto(server_sockfd, (void*)&pkt_send, sizeof(pkt_send), 0, (struct sockaddr *)&client_addr, addr_len);
	cout << "=====Finish=====" << endl <<endl;
}

int detect_loss()
{
	random_device rd;
    	mt19937 gen(rd());
	poisson_distribution<> d(0.1);
	if(d(gen) > 0) return 1;
	else return 2;
}