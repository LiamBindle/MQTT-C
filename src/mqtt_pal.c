#include <mqtt.h>

/** 
 * @file 
 * @brief Implements @ref mqtt_pal_sendall and @ref mqtt_pal_recvall and 
 *        any platform-specific helpers you'd like.
 * @cond Doxygen_Suppress
 */


#ifdef __unix__

#ifdef MQTT_USE_BIO
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

ssize_t mqtt_pal_sendall(mqtt_pal_socket_handle fd, const void* buf, size_t len, int flags) {
    size_t sent = 0;
    while(sent < len) {
        int tmp = BIO_write(fd, buf + sent, len - sent);
        if (tmp > 0) {
            sent += (size_t) tmp;
        } else if (tmp <= 0 && !BIO_should_retry(fd)) {
            return MQTT_ERROR_SOCKET_ERROR;
        }
    }
    
    return sent;
}

ssize_t mqtt_pal_recvall(mqtt_pal_socket_handle fd, void* buf, size_t bufsz, int flags) {
    const void const *start = buf;
    int rv;
    do {
        rv = BIO_read(fd, buf, bufsz);
        if (rv > 0) {
            /* successfully read bytes from the socket */
            buf += rv;
            bufsz -= rv;
        } else if (!BIO_should_retry(fd)) {
            /* an error occurred that wasn't "nothing to read". */
            return MQTT_ERROR_SOCKET_ERROR;
        }
    } while (!BIO_should_read(fd));

    return (ssize_t)(buf - start);
}

#else
#include <errno.h>

ssize_t mqtt_pal_sendall(mqtt_pal_socket_handle fd, const void* buf, size_t len, int flags) {
    size_t sent = 0;
    while(sent < len) {
        ssize_t tmp = send(fd, buf + sent, len - sent, flags);
        if (tmp < 1) {
            return MQTT_ERROR_SOCKET_ERROR;
        }
        sent += (size_t) tmp;
    }
    return sent;
}

ssize_t mqtt_pal_recvall(mqtt_pal_socket_handle fd, void* buf, size_t bufsz, int flags) {
    const void const *start = buf;
    ssize_t rv;
    do {
        rv = recv(fd, buf, bufsz, flags);
        if (rv > 0) {
            /* successfully read bytes from the socket */
            buf += rv;
            bufsz -= rv;
        } else if (rv < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
            /* an error occurred that wasn't "nothing to read". */
            return MQTT_ERROR_SOCKET_ERROR;
        }
    } while (rv > 0);

    return buf - start;
}

#endif

#endif

/** @endcond */