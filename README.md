# MQTT-C
MQTT-C is an [MQTT v3.1.1](http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html) 
client written in C. MQTT is a lightweight publisher-subscriber-based messaging protocol that is
commonly used in IoT and networking applications where high-latency and low data-rate links 
are expected. The purpose of MQTT-C is to provide a **portable** MQTT client, **written in C**, 
for low-power, low-performance devices such as microcontrollers which might or might not be 
running an operating system.

## Getting Started
Please note that pre-built documentation can be found at: `docs/html/index.html`.

To use MQTT-C you must first instantiate a `struct mqtt_client` and initialize it by calling
@ref mqtt_init.
```c
    struct mqtt_client client; /* instantiate the client */
    mqtt_init(&client, ...);   /* initialize the client */
```
Once your client is initialized you must connect to an MQTT broker.
```c
    mqtt_connect(&client, ...); /* send a connection request to the broker. */
```
At this point the client is ready to use! For example, we can subscribe to like so:
```c
    /* subscribe to "toaster/temperature" with a max QoS level of 0 */
    mqtt_subscribe(&client, "toaster/temperature", 0);
```
And we can publish, say the coffee makers temperature, like so:
```c
    /* publish coffee temperature with a QoS level of 1 */
    mqtt_publish(&client, "coffee/temperature", 67, sizeof(int), MQTT_PUBLISH_QOS_1);
```

## Portability
