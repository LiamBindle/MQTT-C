
#include <mqtt.h>

/*
Hi Glenn,

This should be a minimal working unit for you. Comment's that especially need your attention
have been marked with [attn:Glenn]. YOu can grep for these like so: grep -r 'attn'

Let me know if you have questions.

*/


void ingress_callback(void** unused, struct mqtt_response_publish *published); /* callback that is called every time the device receives a message that it's subscribed to. Implemented at the bottom. */

/* Some constant's to get us started. */
#define MQTT_SEND_BUFFER_SIZE 2048 /* size in bytes of your send buffer */
#define MQTT_RECV_BUFFER_SIZE 1024 /* size in bytes of your receive buffer */
#define MQTT_MAX_TOPIC_LENGTH 120  /* the longest topic name you will need*/

int main() 
{
    /* [attn:Glenn] Put your code to initialize your socket here. If you get something like a "file descriptor...replace the "0" below that file 
       descriptor. If there isn't one, that is completely file. This just gets passed to your mqtt_pal_sendall and mqtt_pal_recvall functions (i.e. MQTT-C
       doesn't do ANYTHING with this...it just forwards it along to your PAL functions...so if you don't have FD's just ignore this)
     */
    mqtt_pal_socket_handle sockfd = 0;

    /* place holder for the data you're going to send */
    const char* topic="DevicePublishTopicName";
    const char* application_message = "hello world"; /* application messages are actually void* (i.e. bytes are sent) */
    int application_message_nbytes = 11;

    /* setup a client */
    struct mqtt_client client;
    uint8_t sendbuf[MQTT_SEND_BUFFER_SIZE];
    uint8_t recvbuf[MQTT_RECV_BUFFER_SIZE];

    /* initialize the client */
    mqtt_init(&client, sockfd, sendbuf, MQTT_SEND_BUFFER_SIZE, recvbuf, MQTT_RECV_BUFFER_SIZE, ingress_callback);

    /* connect to the broker */
    mqtt_connect(&client, "publishing_client", (void*)0, (void*)0, 0, (void*)0, (void*)0, 0, 400);

    /* check that we don't have any errors */
    if (client.error != MQTT_OK) {
        return 1; /* error ! */
    }

    while(1) { /* this is your event loop */
        
        /* This is how you publish data */
        mqtt_publish(&client, topic, (void*) application_message, application_message_nbytes, MQTT_PUBLISH_QOS_0);  

        /* This is how you sync I/O. If data is received...the ingress_callback will be called from inside this function. */
        mqtt_sync(&client);

        /* check for errors */
        if (client.error != MQTT_OK) {
            return 1; /* error */
        }     
    }
}


void ingress_callback(void** unused, struct mqtt_response_publish *received) 
{
    /* note that received->topic_name is NOT null-terminated (here we'll change it to a c-string) */
    char received_topic[MQTT_MAX_TOPIC_LENGTH];
    int i;

    for(i = 0; i < received->topic_name_size; i = i + 1) {
        received_topic[i] = ((char*)received->topic_name)[i];
    }
    received_topic[received->topic_name_size] = '\0';

    /* [attn:Glenn] received_topic is a cstring with the topic name of the message that was just received. Add code to handle ingress traffic below.*/

    /*
    i.e.

    if( received_topic is "do this thing") {
        ... handle it appropriately
    } else if ( received_topic is "this other thing" ) {
        ... handle it ...
    }
    */
}