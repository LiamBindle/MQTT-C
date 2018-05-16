
/**
 * @file
 * A simple program that subscribes to a topic.
 */
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <mqtt.h>

struct reconnect_state_t {
    const char* hostname;
    const char* port;
    const char* topic;
};

void reconnect_client(struct mqtt_client* client, void **reconnect_state_vptr);


/**
 * @brief The function will be called whenever a PUBLISH message is received.
 */
void publish_callback(void** unused, struct mqtt_response_publish *published);

/**
 * @brief The client's refresher. This function triggers back-end routines to 
 *        handle ingress/egress traffic to the broker.
 * 
 * @note All this function needs to do is call \ref __mqtt_recv and 
 *       \ref __mqtt_send every so often. I've picked 100 ms meaning that 
 *       client ingress/egress traffic will be handled every 100 ms.
 */
void* client_refresher(void* client);

/**
 * @brief Safelty closes the \p sockfd and cancels the \p client_daemon before \c exit. 
 */
void exit_example(int status, int sockfd, pthread_t *client_daemon);

int main(int argc, const char *argv[]) 
{
    const char* addr;
    const char* port;
    const char* topic;

    /* get address (argv[1] if present) */
    if (argc > 1) {
        addr = argv[1];
    } else {
        addr = "test.mosquitto.org";
    }

    /* get port number (argv[2] if present) */
    if (argc > 2) {
        port = argv[2];
    } else {
        port = "1883";
    }

    /* get the topic name to publish */
    if (argc > 3) {
        topic = argv[3];
    } else {
        topic = "datetime";
    }

    /* build the reconnect_state structure which will be passed to reconnect */
    struct reconnect_state_t reconnect_state;
    reconnect_state.hostname = addr;
    reconnect_state.port = port;
    reconnect_state.topic = topic;

    /* setup a client */
    struct mqtt_client client;
    uint8_t sendbuf[2048]; /* sendbuf should be large enough to hold multiple whole mqtt messages */
    uint8_t recvbuf[1024]; /* recvbuf should be large enough any whole mqtt message expected to be received */
    mqtt_init2(&client, 
               reconnect_client, &reconnect_state, 
               sendbuf, sizeof(sendbuf), 
               recvbuf, sizeof(recvbuf), 
               publish_callback
    );

    /* start a thread to refresh the client (handle egress and ingree client traffic) */
    pthread_t client_daemon;
    if(pthread_create(&client_daemon, NULL, client_refresher, &client)) {
        fprintf(stderr, "Failed to start client daemon.\n");
        exit_example(EXIT_FAILURE, -1, NULL);

    }

    /* start publishing the time */
    printf("%s listening for '%s' messages.\n", argv[0], topic);
    printf("Press ENTER to inject an error.\n");
    printf("Press CTRL-D to exit.\n\n");
    
    /* block */
    while(fgetc(stdin) != EOF) {
        printf("Injecting error: \"MQTT_ERROR_SOCKET_ERROR\"\n");
        client.error = MQTT_ERROR_SOCKET_ERROR;
    } 
    
    /* disconnect */
    printf("\n%s disconnecting from %s\n", argv[0], addr);
    sleep(1);

    /* exit */ 
    exit_example(EXIT_SUCCESS, client.socketfd, &client_daemon);
}

void reconnect_client(struct mqtt_client* client, void **reconnect_state_vptr)
{
    struct reconnect_state_t *reconnect_state = *((struct reconnect_state_t**) reconnect_state_vptr);

    /* Firstly, we should close the clients socket if one is open. */
    if (client->error != MQTT_ERROR_NO_SOCKET) {
        close(client->socketfd);
    }
    
    /* Next, perform error handling/logging. */
    if (client->error != MQTT_ERROR_NO_SOCKET) {
        printf("reconnect_client: called while client was in error state \"%s\"\n", mqtt_error_str(client->error));
    }

    /* Now I'll clear the clients error. */
    client->error = MQTT_OK;

    /* Currently the clients socket is either not present or closed; so we need to make a new one. */
    client->socketfd = mqtt_pal_sockopen(reconnect_state->hostname, reconnect_state->port, AF_INET);
    if (client->socketfd == -1) {
        perror("Failed to open socket: ");
        exit_example(EXIT_FAILURE, client->socketfd, NULL);
    }

    /* Now all normal API calls are valid (except mqtt_init). So lets set the client back up. */
    
    /* Send connection request to the broker. */
    mqtt_connect(client, "subscribing_client", NULL, NULL, 0, NULL, NULL, 0, 400);

    /* Subscribe to the topic. */
    mqtt_subscribe(client, reconnect_state->topic, 0);
}

void exit_example(int status, int sockfd, pthread_t *client_daemon)
{
    if (sockfd != -1) close(sockfd);
    if (client_daemon != NULL) pthread_cancel(*client_daemon);
    exit(status);
}



void publish_callback(void** unused, struct mqtt_response_publish *published) 
{
    /* note that published->topic_name is NOT null-terminated (here we'll change it to a c-string) */
    char* topic_name = (char*) malloc(published->topic_name_size + 1);
    memcpy(topic_name, published->topic_name, published->topic_name_size);
    topic_name[published->topic_name_size] = '\0';

    printf("Received publish('%s'): %s\n", topic_name, (const char*) published->application_message);

    free(topic_name);
}

void* client_refresher(void* client)
{
    while(1) 
    {
        mqtt_sync((struct mqtt_client*) client);
        mqtt_recover((struct mqtt_client*) client);
        usleep(100000U);
    }
    return NULL;
}