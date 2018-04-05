#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>

#include <mqtt.h>
#include <mqtt_client.h>

#define bufsz 256
#define seed 0x1231

struct mqtt_client client;



uint16_t data_gen(uint16_t lfsr){

			int lsb;

			if(lfsr == 0){
				lfsr = seed;
			}

			lsb = lfsr & 1;
			lfsr >>= 1;

			if(lsb){
				lfsr ^= 0xB400u;
			}

			return lfsr;
}

int conf_client(const char* addr, const char* port, const struct addrinfo* hints, struct sockaddr_storage* sockaddr) {
    int sockfd = -1;
    int rv;
    struct addrinfo *p, *servinfo;
    char errbuf[128];

    /* get address information */
    rv = getaddrinfo(addr, port, hints, &servinfo);
    if(rv != 0) {
        fprintf(stderr, "error: %s: line %d: getaddrinfo: %s\n",
            __FILE__, __LINE__ - 3, gai_strerror(rv)
        );
        return -1;
    }

    /* open the first possible socket */
    for(p = servinfo; p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1) {
            sprintf(errbuf, "error: %s: line %d: socket: ", __FILE__, __LINE__ - 2);
            perror(errbuf);
            continue;
        }

        /* connect to server */
        rv = connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);
        if(rv == -1) {
            sprintf(errbuf, "error: %s: line %d: connect: ", __FILE__, __LINE__ - 2);
            perror(errbuf);
            continue;
        }
        break;
    }

    /* memcpy the configured socket info */
    if(sockaddr != NULL) memcpy(sockaddr, p->ai_addr, p->ai_addrlen);

    /* free servinfo */
    freeaddrinfo(servinfo);

    /* return the new socket fd */
    return sockfd;
}

static void publish_response_callback(void **state, struct mqtt_response_publish *publish){

	printf("callback function");
}


static void test_mqtt_init(void** state) {
			const char* addr = "broker.hivemq.com";
			const char* port = "1883";
			int sockfd;
			struct addrinfo hints = {0};
			struct sockaddr_storage sockaddr;
			ssize_t rv;
			struct mqtt_response mqtt_response;
			void(*publish_response_callback_ptr)(void **, struct mqtt_response_publish*);
			uint8_t *sendbuf_ptr, *recvbuf_ptr;

			hints.ai_family = AF_INET;         /* use IPv4 */
			hints.ai_socktype = SOCK_STREAM;    /* TCP */
			sockfd = conf_client(addr, port, &hints, &sockaddr);
			assert_true(sockfd != -1);

			sendbuf_ptr = (uint8_t *) malloc(256);
			recvbuf_ptr = (uint8_t *) malloc(256);
			publish_response_callback_ptr = &publish_response_callback;
			rv = mqtt_init(&client,sockfd, sendbuf_ptr, bufsz, recvbuf_ptr, bufsz, publish_response_callback_ptr);
			assert_true(rv > 0);
			free(sendbuf_ptr);
			free(recvbuf_ptr);
			close(sockfd);
}




static void test_mqtt_connect(){
			const char* addr = "broker.hivemq.com";
			const char* port = "1883";
			int sockfd, i;
			struct addrinfo hints = {0};
			struct sockaddr_storage sockaddr;
			ssize_t rv;
			struct mqtt_response mqtt_response;
			void(*publish_response_callback_ptr)(void **, struct mqtt_response_publish*);
			uint8_t *sendbuf_ptr, *recvbuf_ptr;
			char lfsr_string[40][16]; /*random string numbers to be used for testing*/

			struct valid_packet_s
			{
					char* client_id;
					char* will_topic;
					char* will_message;
					size_t will_message_size;
					char* username;
					char* password;
					uint8_t connect_flags;
					uint16_t keep_alive;
			}valid_packet;

			/*initialize struct*/
			valid_packet.client_id = "demilade"; /*client_id*/
			valid_packet.will_topic  ="last_words"; /*will topic*/
			valid_packet.will_message = "goodbye";/*will message*/
			valid_packet.will_message_size = 10;/*will message size*/
			valid_packet.username = "demilade10";/*username*/
			valid_packet.password = "pass";/*password*/
			valid_packet.connect_flags = 0xE4;/*11110100*/
			valid_packet.keep_alive = 10;/*keep_alive*/


			hints.ai_family = AF_INET;         /* use IPv4 */
			hints.ai_socktype = SOCK_STREAM;    /* TCP */
			uint16_t lfsr;

			lfsr = data_gen(seed);
			for(i = 0; i < 40; i++){
						sprintf(lfsr_string[i], "%d", lfsr);
						lfsr = data_gen(lfsr);

			}

			sockfd = conf_client(addr, port, &hints, &sockaddr);
			sendbuf_ptr = (uint8_t *) malloc(256);
			recvbuf_ptr = (uint8_t *) malloc(256);
			publish_response_callback_ptr = &publish_response_callback;
			mqtt_init(&client,sockfd, sendbuf_ptr, bufsz, recvbuf_ptr, bufsz, publish_response_callback_ptr);

			/*test client field is mandatory. All fields are valid except for client ID*/
			rv = mqtt_connect(&client, valid_packet.client_id,valid_packet.will_topic, valid_packet.will_message,
												valid_packet.will_message_size, valid_packet.username, valid_packet.password,
												valid_packet.connect_flags, valid_packet.keep_alive);

			printf("rv: %s\n",mqtt_error_str(rv));
			assert_true(rv > 0);
			/*test client field is mandatory. All fields are valid except for client ID*/
			// mqtt_init(&client,sockfd, sendbuf_ptr, bufsz, recvbuf_ptr, bufsz, publish_response_callback_ptr);
			// rv = mqtt_connect(&client, valid_packet.client_id,valid_packet.will_topic, valid_packet.will_message,
			// 									valid_packet.will_message_size, valid_packet.username, valid_packet.password,
			// 									valid_packet.connect_flags, valid_packet.keep_alive);
			// printf("rv: %s\n",mqtt_error_str(rv));
			// assert_true(strcmp(mqtt_error_str(rv), "MQTT_ERROR_CONNECT_NULL_CLIENT_ID") == 0);

			free(sendbuf_ptr);
			free(recvbuf_ptr);
			close(sockfd);
}

int main(){

			const struct CMUnitTest tests[] = {
			    	cmocka_unit_test(test_mqtt_init),
						cmocka_unit_test(test_mqtt_connect),
			};

			return cmocka_run_group_tests(tests, NULL, NULL);
}
