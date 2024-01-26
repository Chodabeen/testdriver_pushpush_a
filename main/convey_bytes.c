#include <stdio.h>

int recv_bytes(int sock_fd, void * buf, size_t len);
int send_bytes(int sock_fd, void * buf, size_t len);
	

int main(int argc, char * argv[]) {
    char buf[4095];

	int size = 3;
    
    size = recv_bytes(0, buf, sizeof(buf));
	printf("size: %d\n", size);

    send_bytes(1, buf, sizeof(buf));
    
}