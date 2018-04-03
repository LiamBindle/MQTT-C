#ifndef __MQTT_PAL_H__
#define __MQTT_PAL_H__

#include <arpa/inet.h>  //< htons ntohs


ssize_t mqtt_pal_sendall(int fd, void* buf, size_t bufsz, int flags);
ssize_t mqtt_pal_recvall(int fd, void* buf, size_t bufsz, int flags);

#endif