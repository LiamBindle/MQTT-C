/*
MIT License

Copyright(c) 2018 Liam Bindle

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <mqtt.h>

/* [attn:Glenn] You'll have to implement this function. It's job is to send all the bytes in buf (number of bytes to send is len).
    You can ignore flags...those aren't used.
    If you don't have a socket fd...you can ignore it. fd is just whatever you passed to mqtt_init initially (again...it's not touched by MQTT-C internals).
    Return the number of bytes sent (IMPORTANT).
*/
ssize_t mqtt_pal_sendall(mqtt_pal_socket_handle fd, const void* buf, size_t len, int flags) {
    /* below is the implementation for POSIX-like sockets for your reference */

    size_t sent = 0;
    while(sent < len) {
        ssize_t tmp = 0; /* "0" used to be: send(fd, buf + sent, len - sent, flags);*/
        if (tmp < 1) {
            return MQTT_ERROR_SOCKET_ERROR;
        }
        sent += (size_t) tmp;
    }
    return sent;
}

/* [attn:Glenn] You'll have to implement this function. It's job is to receive bytes from the network and put them in buf.
    If the number of bytes to receive is LARGER than bufsz (the size of the buffer your're writing to)...you should return MQTT_ERROR_SOCKET_ERROR.
    Return the number of bytes written to buf (IMPORTANT). 
*/
ssize_t mqtt_pal_recvall(mqtt_pal_socket_handle fd, void* buf, size_t bufsz, int flags) {
    const void const *start = buf;
    ssize_t rv;
    do {
        rv = 0; /* "0" used to be: recv(fd, buf, bufsz, flags); */
        if (rv > 0) {
            /* successfully read bytes from the socket */
            buf += rv;
            bufsz -= rv;
        } else if (rv < 0 /*&& errno != EAGAIN && errno != EWOULDBLOCK*/) {
            /* an error occurred that wasn't "nothing to read". */
            return MQTT_ERROR_SOCKET_ERROR;
        }
    } while (rv > 0);

    return buf - start;
}