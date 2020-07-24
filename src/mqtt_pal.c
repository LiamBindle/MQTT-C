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

#include <mqttc.h>

/** 
 * @file 
 * @brief Implements @ref mqtt_pal_sendall and @ref mqtt_pal_recvall and 
 *        any platform-specific helpers you'd like.
 * @cond Doxygen_Suppress
 */


#ifdef MQTT_USE_MBEDTLS
#include <mbedtls/ssl.h>

ssize_t mqtt_pal_sendall(mqtt_pal_socket_handle fd, const void* buf, size_t len, int flags) {
    size_t sent = 0;
    while(sent < len) {
        int rv = mbedtls_ssl_write(fd, buf + sent, len - sent);
        if (rv < 0) {
            if (rv == MBEDTLS_ERR_SSL_WANT_READ ||
                rv == MBEDTLS_ERR_SSL_WANT_WRITE
#if defined(MBEDTLS_ERR_SSL_ASYNC_IN_PROGRESS)
                || rv == MBEDTLS_ERR_SSL_ASYNC_IN_PROGRESS
#endif
#if defined(MBEDTLS_ERR_SSL_CRYPTO_IN_PROGRESS)
                || rv == MBEDTLS_ERR_SSL_CRYPTO_IN_PROGRESS
#endif
                ) {
                /* should call mbedtls_ssl_writer later again */
                break;
            }
            return MQTT_ERROR_SOCKET_ERROR;
        }
        sent += (size_t) rv;
    }
    return sent;
}

ssize_t mqtt_pal_recvall(mqtt_pal_socket_handle fd, void* buf, size_t bufsz, int flags) {
    const void *const start = buf;
    int rv;
    do {
        rv = mbedtls_ssl_read(fd, buf, bufsz);
        if (rv < 0) {
            if (rv == MBEDTLS_ERR_SSL_WANT_READ ||
                rv == MBEDTLS_ERR_SSL_WANT_WRITE
#if defined(MBEDTLS_ERR_SSL_ASYNC_IN_PROGRESS)
                || rv == MBEDTLS_ERR_SSL_ASYNC_IN_PROGRESS
#endif
#if defined(MBEDTLS_ERR_SSL_CRYPTO_IN_PROGRESS)
                || rv == MBEDTLS_ERR_SSL_CRYPTO_IN_PROGRESS
#endif
                ) {
                /* should call mbedtls_ssl_read later again */
                break;
            }
            return MQTT_ERROR_SOCKET_ERROR;
        }
        buf = (char*)buf + rv;
        bufsz -= rv;
    } while (rv > 0);

    return buf - start;
}

#elif defined(MQTT_USE_BEARSSL)
#include <bearssl.h>
#include <memory.h>

static int do_rec_data(mqtt_pal_socket_handle fd, unsigned int status) {
    ssize_t rc;
    uint8_t *buffer;
    size_t length;
    int err;

    err = br_ssl_engine_last_error(&fd->sc.eng);

    if (err != BR_ERR_OK) {
        return MQTT_ERROR_SOCKET_ERROR;
    }

    if ((status & BR_SSL_SENDREC) == BR_SSL_SENDREC) {
        buffer = br_ssl_engine_sendrec_buf(&fd->sc.eng, &length);

        if (length > 0) {
            if ((rc = fd->low_write(&fd->fd, buffer, length)) < 0) {
                return MQTT_ERROR_SOCKET_ERROR;
            }

            br_ssl_engine_sendrec_ack(&fd->sc.eng, rc);
        }
    }
    else if ((status & BR_SSL_RECVREC) == BR_SSL_RECVREC) {
        buffer = br_ssl_engine_recvrec_buf(&fd->sc.eng, &length);

        if (length > 0) {
            if ((rc = fd->low_read(&fd->fd, buffer, length)) < 0) {
                return MQTT_ERROR_SOCKET_ERROR;
            }
            
            br_ssl_engine_recvrec_ack(&fd->sc.eng, rc);
        }
    }
    else if ((status && BR_SSL_CLOSED) == BR_SSL_CLOSED) {
        return MQTT_ERROR_SOCKET_ERROR;
    }

    return MQTT_OK;
}

ssize_t mqtt_pal_sendall(mqtt_pal_socket_handle fd, const void* buf, size_t len, int flags) {
    int rc;
    uint8_t *buffer;
    size_t length;
    size_t remaining_bytes = len;
    const uint8_t *walker = buf;
    unsigned int status;

    while (remaining_bytes > 0) {

        if (rc == MQTT_ERROR_SOCKET_ERROR) {
            return rc;
        }

        status = br_ssl_engine_current_state(&fd->sc.eng);

        if ((status & BR_SSL_CLOSED) != 0) {
            return MQTT_ERROR_SOCKET_ERROR;
        }

        if ((status & (BR_SSL_RECVREC | BR_SSL_SENDREC)) != 0) {
            rc = do_rec_data(fd, status);

            if (rc != MQTT_OK) {
                return rc;
            }
            status = br_ssl_engine_current_state(&fd->sc.eng);
        }

        if ((status & BR_SSL_SENDAPP) == BR_SSL_SENDAPP) {
            buffer = br_ssl_engine_sendapp_buf(&fd->sc.eng, &length);

            if (length > 0) {
                size_t write = length >= remaining_bytes? remaining_bytes : length;
                memcpy(buffer, walker, write);
                remaining_bytes -= write;
                walker += write;
                br_ssl_engine_sendapp_ack(&fd->sc.eng, write);
                br_ssl_engine_flush(&fd->sc.eng, 0);
            }
        }
    }

    return len;
}

ssize_t mqtt_pal_recvall(mqtt_pal_socket_handle fd, void* buf, size_t bufsz, int flags) {
    int rc;
    uint8_t *buffer;
    size_t length;
    size_t remaining_bytes = bufsz;
    uint8_t *walker = buf;
    unsigned int status;

    if (rc == MQTT_ERROR_SOCKET_ERROR) {
        return rc;
    }

    status = br_ssl_engine_current_state(&fd->sc.eng);

    if ((status & (BR_SSL_RECVREC | BR_SSL_SENDREC)) != 0) {
        rc = do_rec_data(fd, status);

        if (rc != MQTT_OK) {
            return rc;
        }
        status = br_ssl_engine_current_state(&fd->sc.eng);
    }

    if ((status & BR_SSL_RECVAPP) == BR_SSL_RECVAPP) {
        buffer = br_ssl_engine_recvapp_buf(&fd->sc.eng, &length);

        if (length > 0) {
            size_t write = length >= remaining_bytes? remaining_bytes : length;
            memcpy(walker, buffer, write);
            remaining_bytes -= write;
            walker += write;
            br_ssl_engine_recvapp_ack(&fd->sc.eng, write);
        }
    }

    return bufsz - remaining_bytes;
}

#elif defined(MQTT_USE_BIO)
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
    const void *const start = buf;
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

#elif defined(__unix__) || defined(__APPLE__)

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
    const void *const start = buf;
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

#elif defined(_MSC_VER)

#include <errno.h>

ssize_t mqtt_pal_sendall(mqtt_pal_socket_handle fd, const void* buf, size_t len, int flags) {
    size_t sent = 0;
    while(sent < len) {
        ssize_t tmp = send(fd, (char*)buf + sent, len - sent, flags);
        if (tmp < 1) {
            return MQTT_ERROR_SOCKET_ERROR;
        }
        sent += (size_t) tmp;
    }
    return sent;
}

ssize_t mqtt_pal_recvall(mqtt_pal_socket_handle fd, void* buf, size_t bufsz, int flags) {
    const char *const start = buf;
    ssize_t rv;
    do {
        rv = recv(fd, buf, bufsz, flags);
        if (rv > 0) {
            /* successfully read bytes from the socket */
            buf = (char*)buf + rv;
            bufsz -= rv;
        } else if (rv < 0) {
            int err = WSAGetLastError();
            if (err != WSAEWOULDBLOCK) {
                /* an error occurred that wasn't "nothing to read". */
                return MQTT_ERROR_SOCKET_ERROR;
            }
        }
    } while (rv > 0);

    return (ssize_t)((char*)buf - start);
}

#else

#error No PAL!

#endif

/** @endcond */
