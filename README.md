# MQTT-C
MQTT-C is an [MQTT v3.1.1](http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html) 
client written in C. MQTT is a lightweight publisher-subscriber-based messaging protocol that is
commonly used in IoT and networking applications where high-latency and low data-rate links 
are expected. The purpose of MQTT-C is to provide a **portable** MQTT client, **written in C**, 
for low-power, low-performance devices such as microcontrollers which might or might not be 
running an operating system.

## Getting Started
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
    int temperature = 67;
    mqtt_publish(&client, "coffee/temperature", &temperature, sizeof(int), MQTT_PUBLISH_QOS_1);
```

## Building
There are **only two source files** that need to be built, `mqtt.c` and `mqtt_pal.c`.
You should be able to build these files with any **C99 (or more recent) compilers**.

Then, simply include `mqtt.h`.

## Documentation
Pre-built documentation can be found at: `"docs/html/index.html"`. 

The @ref api documentation contains all the MQTT-C API documentation. Other modules contain
documentation for MQTT-C developers.

## The Fundamentals

## Testing and Building the Tests
The MQTT-C unit tests makes use of the [cmocka unit testing framework](https://cmocka.org/). 
Therefore, [cmocka](https://cmocka.org/) *must* be installed on your machine in order to 
build and run the unit tests. For convenience we have included a simple makefile to build 
the MQTT-C unit tests. The tests can be built as follows:
```bash
    $ make all
``` 
And then run like so:
```bash
    $ ./tests
```
Note that these unit tests make use of the 
[Mosquitto MQTT Test Server](https://test.mosquitto.org/) which is a free service and is sometime
down in which case some tests will fail.

## Portability
MQTT-C provides a transparent platform abstraction layer (PAL) in `mqtt_pal.h` and `mqtt_pal.c`.
These files declare and describe the types and system-calls that MQTT-C requires. Refer to 
@ref pal for the complete documentation of the PAL.

## License
This project is licensed under the [MIT License](https://opensource.org/licenses/MIT). See the 
`"LICENSE"` file for more details.

## Authors
MQTT-C was initially developed as a CMPT 434 (Winter Term, 2018) final project at the University of 
Saskatchewan by:
- **Demilade Adeoye**
- **Liam Bindle**

