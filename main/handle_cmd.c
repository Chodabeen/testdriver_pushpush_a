#include <stdio.h>
#include <pthread.h>
#include "client.h"

int main(int argc, char * argv[]) {

    /* handle_cmd and recv_msg */
    pthread_t rcv_thread;
    void * thread_return;

	pthread_mutex_init(&mutx, NULL);
	pthread_cond_init(&cond, NULL);

    int stdin_sock = 0;
    pthread_create(&rcv_thread, NULL, recv_msg, (void*)&stdin_sock);
    
	int i;
	for(i = 0; i < 16; i++) {
		handle_cmd(NULL);
	}

}