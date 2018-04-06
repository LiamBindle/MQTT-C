# MQTT-C
MQTT-C is an [MQTT v3.1.1](http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html) 
client written in C. MQTT is a lightweight publisher-subscriber-based messaging protocol that is
commonly used in IoT and networking applications where high-latency and low data-rate links 
are expected. The purpose of MQTT-C is to provide a **portable** MQTT client, **written in C**, 
for embedded systems and PC's alike. MQTT-C does this by providing a transparent Platform 
Abstraction Layer (PAL) which makes porting to new platforms easy. MQTT-C is completely 
thread-safe but can also run perfectly fine on single-threaded systems making MQTT-C 
well-suited for embedded systems and microcontrollers.

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
And we can publish, say the coffee maker's temperature, like so:
```c
    /* publish coffee temperature with a QoS level of 1 */
    int temperature = 67;
    mqtt_publish(&client, "coffee/temperature", &temperature, sizeof(int), MQTT_PUBLISH_QOS_1);
```

## Building
There are **only two source files** that need to be built, `mqtt.c` and `mqtt_pal.c`.
You should be able to build these files with any **C99 (or more recent) compilers**.

Then, simply <code>\#include <mqtt.h></code>.

## Documentation
Pre-built documentation can be found at `"docs/index.html"`, or online 
[here](https://liambindle.ca/MQTT-C). 

The @ref api documentation contains all the documentation application programmers should need. 
The @ref pal documentation contains everything you should need to port MQTT-C to a new platform,
and the other modules contain documentation for MQTT-C developers.

## Testing and Building the Tests
The MQTT-C unit tests use the [cmocka unit testing framework](https://cmocka.org/). 
Therefore, [cmocka](https://cmocka.org/) *must* be installed on your machine to build and run 
the unit tests. For convenience, a simple `"makefile"` is included to build the unit tests and 
examples on UNIX-like machines. The unit tests and examples can be built as follows:
```bash
    $ make all
``` 
The unit tests and examples will be built in the `"bin/"` directory. The unit tests can be run 
like so:
```bash
    $ ./bin/tests [address [port]]
```
Note that the \c address and \c port arguments are both optional to specify the location of the
MQTT broker that is to be used for the tests. If no \c address is given then the 
[Mosquitto MQTT Test Server](https://test.mosquitto.org/) will be used. If no \c port is given, 
port 1883 will be used.

## Portability
MQTT-C provides a transparent platform abstraction layer (PAL) in `mqtt_pal.h` and `mqtt_pal.c`.
These files declare and implement the types and calls that MQTT-C requires. Refer to 
@ref pal for the complete documentation of the PAL.

## Contributing
Please feel free to submit issues and pull-requests [here](https://github.com/LiamBindle/MQTT-C).
When submitting a pull-request please ensure you have *fully documented* your changes and 
added the appropriate unit tests.


## License
This project is licensed under the [MIT License](https://opensource.org/licenses/MIT). See the 
`"LICENSE"` file for more details.

## Authors
MQTT-C was initially developed as a CMPT 434 (Winter Term, 2018) final project at the University of 
Saskatchewan by:
- **Liam Bindle**
- **Demilade Adeoye**

