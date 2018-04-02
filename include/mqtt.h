#ifndef __MQTT_H__
#define __MQTT_H__

#include <stdint.h>     //< uintXX_t ...
#include <stddef.h>     //< size_t
#include <string.h>     //< strlen ...
#include <limits.h>     //< INT_MIN
#include <sys/types.h>  //< ssize_t
#include <arpa/inet.h>  //< htons ntohs
#include <stdarg.h>

/**
 * @file
 * 
 * @mainpage 
 * Welcome 
 * 
 * @defgroup api API
 * @brief Application programmer interface documentation.
 * 
 * @defgroup packers Control Packet Serialization
 * @brief Documentation of functions and datastructures for MQTT control packet serialization.
 * 
 * @defgroup unpackers Control Packet Deserialization
 * @brief Documentation of functions and datastructures for MQTT control packet deserialization.
 *
 * @note To deserialize a packet from a buffer use \ref mqtt_unpack_response (it's the only 
 *       function you need).
 */


 /**
  * @brief An enumeration of the MQTT control packet types. 
  * @ingroup unpackers
  *
  * @see <a href="http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718021">
  * MQTT v3.1.1: MQTT Control Packet Types
  * </a>
  */
  enum MQTTControlPacketType {
    MQTT_CONTROL_CONNECT=1u,
    MQTT_CONTROL_CONNACK=2u,
    MQTT_CONTROL_PUBLISH=3u,
    MQTT_CONTROL_PUBACK=4u,
    MQTT_CONTROL_PUBREC=5u,
    MQTT_CONTROL_PUBREL=6u,
    MQTT_CONTROL_PUBCOMP=7u,
    MQTT_CONTROL_SUBSCRIBE=8u,
    MQTT_CONTROL_SUBACK=9u,
    MQTT_CONTROL_UNSUBSCRIBE=10u,
    MQTT_CONTROL_UNSUBACK=11u,
    MQTT_CONTROL_PINGREQ=12u,
    MQTT_CONTROL_PINGRESP=13u,
    MQTT_CONTROL_DISCONNECT=14u
};

/**
 * @brief The fixed header of an MQTT control packet.
 * @ingroup unpackers
 * 
 * @see <a href="http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718020">
 * MQTT v3.1.1: Fixed Header
 * </a>
 */
struct mqtt_fixed_header {
    /** The type of packet. */
    enum MQTTControlPacketType control_type;

    /** The packets control flags.*/
    uint8_t control_flags: 4;

    /** The remaining size of the packet in bytes (i.e. the size of variable header and payload).*/
    uint32_t remaining_length;
};

/**
 * @brief The protocol identifier for MQTT v3.1.1.
 * @ingroup packers
 * 
 * @see <a href="http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718030">
 * MQTT v3.1.1: CONNECT Variable Header.
 * </a>  
 */
#define MQTT_PROTOCOL_LEVEL 0x04

/** 
 * @brief A macro used to declare the enum MqttErrors and associated 
 *        error messages (the members of the num) at the same time.
 */
#define __ALL_MQTT_ERRORS(MQTT_ERROR)                    \
    MQTT_ERROR(MQTT_ERROR_NULLPTR)                       \
    MQTT_ERROR(MQTT_ERROR_CONTROL_FORBIDDEN_TYPE)        \
    MQTT_ERROR(MQTT_ERROR_CONTROL_INVALID_FLAGS)         \
    MQTT_ERROR(MQTT_ERROR_CONTROL_WRONG_TYPE)            \
    MQTT_ERROR(MQTT_ERROR_CONNECT_NULL_CLIENT_ID)        \
    MQTT_ERROR(MQTT_ERROR_CONNECT_NULL_WILL_MESSAGE)     \
    MQTT_ERROR(MQTT_ERROR_CONNECT_FORBIDDEN_WILL_QOS)    \
    MQTT_ERROR(MQTT_ERROR_CONNACK_FORBIDDEN_FLAGS)       \
    MQTT_ERROR(MQTT_ERROR_CONNACK_FORBIDDEN_CODE)        \
    MQTT_ERROR(MQTT_ERROR_PUBLISH_FORBIDDEN_QOS)         \
    MQTT_ERROR(MQTT_ERROR_SUBSCRIBE_TOO_MANY_TOPICS)     \
    MQTT_ERROR(MQTT_ERROR_MALFORMED_RESPONSE)            \
    MQTT_ERROR(MQTT_ERROR_UNSUBSCRIBE_TOO_MANY_TOPICS)   \
    MQTT_ERROR(MQTT_ERROR_RESPONSE_INVALID_CONTROL_TYPE) \
    MQTT_ERROR(MQTT_ERROR_CLIENT_NOT_CONNECTED)          \
    MQTT_ERROR(MQTT_ERROR_SEND_BUFFER_IS_FULL)           \
    MQTT_ERROR(MQTT_ERROR_SOCKET_ERROR)                  \
    MQTT_ERROR(MQTT_ERROR_MALFORMED_REQUEST)             \
    MQTT_ERROR(MQTT_ERROR_RECV_BUFFER_TOO_SMALL)         \
    MQTT_ERROR(MQTT_ERROR_ACK_OF_UNKNOWN)                \
    MQTT_ERROR(MQTT_ERROR_NOT_IMPLEMENTED)               \
    MQTT_ERROR(MQTT_ERROR_CONNECTION_REFUSED)            \
    MQTT_ERROR(MQTT_ERROR_SUBSCRIBE_FAILED)              \

/* todo: add more connection refused errors */

/** 
 * @brief A macro used to generate the enum MqttErrors from 
 *        \ref __ALL_MQTT_ERRORS
 * @see __ALL_MQTT_ERRORS
*/
#define GENERATE_ENUM(ENUM) ENUM,

/** 
 * @brief A macro used to generate the error messages associated with 
 *        MqttErrors from \ref __ALL_MQTT_ERRORS
 * @see __ALL_MQTT_ERRORS
*/
#define GENERATE_STRING(STRING) #STRING,


/** 
 * @brief An enumeration of error codes. Error messages can be retrieved by calling \ref mqtt_error_str.
 * @ingroup api
 * 
 * @see mqtt_error_str
 */
enum MqttErrors {
    MQTT_ERROR_UNKNOWN=INT_MIN,
    __ALL_MQTT_ERRORS(GENERATE_ENUM)
    MQTT_OK = 1
};

/** 
 * @brief Returns an error message for error code, \p error.
 * @ingroup api
 * 
 * @param[in] error the error code.
 * 
 * @returns The associated error message.
 */
const char* mqtt_error_str(enum MqttErrors error);

/**
 * @brief Pack a MQTT string, given a c-string \p str.
 * 
 * @param[out] buf the buffer that the MQTT string will be written to.
 * @param[in] str the c-string to be written to \p buf.
 * 
 * @warning This function provides no error checking.
 * 
 * @returns strlen(str) + 2
*/
ssize_t __mqtt_pack_str(uint8_t *buf, const char* str);

/** @brief A macro to get the MQTT string length from a c-string. */
#define __mqtt_packed_cstrlen(x) (2 + strlen(x))

/* RESPONSES */

/**
 * @brief An enumeration of the return codes returned in a CONNACK packet.
 * @ingroup unpackers
 * 
 * @see <a href="http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Table_3.1_-">
 * MQTT v3.1.1: CONNACK return codes.
 * </a> 
 */
enum MQTTConnackReturnCode {
    MQTT_CONNACK_ACCEPTED = 0u,
    MQTT_CONNACK_REFUSED_PROTOCOL_VERSION = 1u,
    MQTT_CONNACK_REFUSED_IDENTIFIER_REJECTED = 2u,
    MQTT_CONNACK_REFUSED_SERVER_UNAVAILABLE = 3u,
    MQTT_CONNACK_REFUSED_BAD_USER_NAME_OR_PASSWORD = 4u,
    MQTT_CONNACK_REFUSED_NOT_AUTHORIZED = 5u
};

/**
 * @brief A connection response datastructure.
 * @ingroup unpackers
 * 
 * @see <a href="http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718033">
 * MQTT v3.1.1: CONNACK - Acknowledgement connection response.
 * </a>
 */
struct mqtt_response_connack {
    /** 
     * @brief Allows client and broker to check if they have a consistent view about whether there is
     * already a stored session state.
    */
    uint8_t session_present_flag;

    /** 
     * @brief The return code of the connection request. 
     * 
     * @see MQTTConnackReturnCode
     */
    enum MQTTConnackReturnCode return_code;
};

 /**
  * @brief A publish packet received from the broker.
  * @ingroup unpackers
  * 
  * A publish packet is received from the broker when a client publishes to a topic that the 
  * \em {local client} is subscribed to.
  *
  * @see <a href="http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718037"> 
  * MQTT v3.1.1: PUBLISH - Publish Message.
  * </a> 
  */
struct mqtt_response_publish {
    /** 
     * @brief The DUP flag. DUP flag is 0 if its the first attempt to send this publish packet. A DUP flag
     * of 1 means that this might be a re-delivery of the packet.
     */
    uint8_t dup_flag;

    /** 
     * @brief The quality of service level.
     * 
     * @see <a href="http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Table_3.11_-">
     * MQTT v3.1.1: QoS Definitions
     * </a>
     */
    uint8_t qos_level;

    /** @brief The retain flag of this publish message. */
    uint8_t retain_flag;

    /** @brief Size of the topic name (number of characters). */
    uint16_t topic_name_size;

    /** 
     * @brief The topic name. 
     * @note topic_name is not null terminated. Therefore topic_name_size must be used to get the 
     *       string length.
     */
    const void* topic_name;

    /** @brief The publish message's packet ID. */
    uint16_t packet_id;

    /** @brief The publish message's application message.*/
    const void* application_message;

    /** @brief The size of the application message in bytes. */
    size_t application_message_size;
};

/**
 * @brief A publish acknowledgement for messages that were published with QoS level 1.
 * @ingroup unpackers
 * 
 * @see <a href="http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718043">
 * MQTT v3.1.1: PUBACK - Publish Acknowledgement.
 * </a> 
 *
 */
struct mqtt_response_puback {
    /** @brief The published messages packet ID. */
    uint16_t packet_id;
};

/**
 * @brief The response packet to a PUBLISH packet with QoS level 2.
 * @ingroup unpackers
 * 
 * @see <a href="http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718048">
 * MQTT v3.1.1: PUBREC - Publish Received.
 * </a> 
 *
 */
struct mqtt_response_pubrec {
    /** @brief The published messages packet ID. */
    uint16_t packet_id;
};

/**
 * @brief The response to a PUBREC packet.
 * @ingroup unpackers
 * 
 * @see <a href="http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718053">
 * MQTT v3.1.1: PUBREL - Publish Release.
 * </a> 
 *
 */
struct mqtt_response_pubrel {
    /** @brief The published messages packet ID. */
    uint16_t packet_id;
};

/**
 * @brief The response to a PUBREL packet.
 * @ingroup unpackers
 * 
 * @see <a href="http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718058">
 * MQTT v3.1.1: PUBCOMP - Publish Complete.
 * </a> 
 *
 */
struct mqtt_response_pubcomp {
    /** T@brief he published messages packet ID. */
    uint16_t packet_id;
};

/**
 * @brief An enumeration of subscription acknowledgement return codes.
 * @ingroup unpackers
 * 
 * @see <a href="http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Figure_3.26_-">
 * MQTT v3.1.1: SUBACK Return Codes.
 * </a> 
 */
enum MQTTSubackReturnCodes {
    MQTT_SUBACK_SUCCESS_MAX_QOS_0 = 0u,
    MQTT_SUBACK_SUCCESS_MAX_QOS_1 = 1u,
    MQTT_SUBACK_SUCCESS_MAX_QOS_2 = 2u,
    MQTT_SUBACK_FAILURE           = 128u
};

/**
 * @brief The response to a subscription request.
 * @ingroup unpackers
 * 
 * @see <a href="http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718068">
 * MQTT v3.1.1: SUBACK - Subscription Acknowledgement.
 * </a> 
 */
struct mqtt_response_suback {
    /** @brief The published messages packet ID. */
    uint16_t packet_id;

    /** 
     * Array of return codes corresponding to the requested subscribe topics.
     * 
     * @see MQTTSubackReturnCodes
     */
    const uint8_t *return_codes;

    /** The number of return codes. */
    size_t num_return_codes;
};

/**
 * @brief The brokers response to a UNSUBSCRIBE request.
 * @ingroup unpackers
 * 
 * @see <a href="http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718077">
 * MQTT v3.1.1: UNSUBACK - Unsubscribe Acknowledgement.
 * </a> 
 */
struct mqtt_response_unsuback {
    /** @brief The published messages packet ID. */
    uint16_t packet_id;
};

/**
 * @brief The response to a ping request.
 * @ingroup unpackers
 * 
 * @note This response contains no members.
 * 
 * @see <a href="http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718086">
 * MQTT v3.1.1: PINGRESP - Ping Response.
 * </a> 
 */
struct mqtt_response_pingresp {};

/**
 * @brief A struct used to deserialize/interpret an incoming packet from the broker.
 * @ingroup unpackers
 */
struct mqtt_response {
    /** @brief The mqtt_fixed_header of the deserialized packet. */
    struct mqtt_fixed_header fixed_header;

    /**
     * @brief A union of the possible responses from the broker.
     * 
     * @note The fixed_header contains the control type. This control type corresponds to the
     *       member of this union that should be accessed. For example if 
     *       fixed_header#control_type == \c MQTT_CONTROL_PUBLISH then 
     *       decoded#publish should be accessed.
     */
    union {
        struct mqtt_response_connack  connack;
        struct mqtt_response_publish  publish;
        struct mqtt_response_puback   puback;
        struct mqtt_response_pubrec   pubrec;
        struct mqtt_response_pubrel   pubrel;
        struct mqtt_response_pubcomp  pubcomp;
        struct mqtt_response_suback   suback;
        struct mqtt_response_unsuback unsuback;
        struct mqtt_response_pingresp pingresp;
    } decoded;
};

/**
 * @brief Deserialize the contents of \p buf into an mqtt_fixed_header object.
 * @ingroup unpackers
 * 
 * @note This function performs complete error checking and a positive return value
 *       means the entire mqtt_response can be deserialized from \p buf.
 * 
 * @param[out] response the response who's \ref mqtt_response.fixed_header will be initialized.
 * @param[in] buf the buffer.
 * @param[in] bufsz the total number of bytes in the buffer.
 * 
 * @returns The number of bytes that were consumed, or 0 if the buffer does not contain enough 
 *          bytes to parse the packet, or a negative value if there was a protocol violation.
 */
ssize_t mqtt_unpack_fixed_header(struct mqtt_response *response, const uint8_t *buf, size_t bufsz);

/**
 * @brief Deserialize a CONNACK response from \p buf.
 * @ingroup unpackers
 * 
 * @pre \ref mqtt_unpack_fixed_header must have returned a positive value and the control packet type
 *      must be \c MQTT_CONTROL_CONNACK.
 * 
 * @param[out] mqtt_response the mqtt_response that will be initialized.
 * @param[in] buf the buffer that contains the variable header and payload of the packet. The 
 *                first byte of \p buf should be the first byte of the variable header.
 * 
 * @relates mqtt_response_connack 
 * 
 * @returns The number of bytes that were consumed, or 0 if the buffer does not contain enough 
 *          bytes to parse the packet, or a negative value if there was a protocol violation.
 */
ssize_t mqtt_unpack_connack_response (struct mqtt_response *mqtt_response, const uint8_t *buf);

/**
 * @brief Deserialize a publish response from \p buf.
 * @ingroup unpackers
 * 
 * @pre \ref mqtt_unpack_fixed_header must have returned a positive value and the mqtt_response must
 *      have a control type of \c MQTT_CONTROL_PUBLISH.
 * 
 * @param[out] mqtt_response the response that is initialized from the contents of \p buf.
 * @param[in] buf the buffer with the incoming data.
 * 
 * @relates mqtt_response_publish 
 * 
 * @returns The number of bytes that were consumed, or 0 if the buffer does not contain enough 
 *          bytes to parse the packet, or a negative value if there was a protocol violation.
 */
ssize_t mqtt_unpack_publish_response (struct mqtt_response *mqtt_response, const uint8_t *buf);

/**
 * @brief Deserialize a PUBACK/PUBREC/PUBREL/PUBCOMP packet from \p buf.
 * @ingroup unpackers
 * 
 * @pre \ref mqtt_unpack_fixed_header must have returned a positive value and the mqtt_response must
 *      have a control type of \c MQTT_CONTROL_PUBACK, \c MQTT_CONTROL_PUBREC, \c MQTT_CONTROL_PUBREL
 *      or \c MQTT_CONTROL_PUBCOMP.
 * 
 * @param[out] mqtt_response the response that is initialized from the contents of \p buf.
 * @param[in] buf the buffer with the incoming data.
 *
 * @relates mqtt_response_puback mqtt_response_pubrec mqtt_response_pubrel mqtt_response_pubcomp
 * 
 * @returns The number of bytes that were consumed, or 0 if the buffer does not contain enough 
 *          bytes to parse the packet, or a negative value if there was a protocol violation.
 */
ssize_t mqtt_unpack_pubxxx_response(struct mqtt_response *mqtt_response, const uint8_t *buf);

/**
 * @brief Deserialize a SUBACK packet from \p buf.
 * @ingroup unpacker
 *  
 * @pre \ref mqtt_unpack_fixed_header must have returned a positive value and the mqtt_response must
 *      have a control type of \c MQTT_CONTROL_SUBACK.
 * 
 * @param[out] mqtt_response the response that is initialized from the contents of \p buf.
 * @param[in] buf the buffer with the incoming data.
 *
 * @relates mqtt_response_suback
 * 
 * @returns The number of bytes that were consumed, or 0 if the buffer does not contain enough 
 *          bytes to parse the packet, or a negative value if there was a protocol violation.
 */
ssize_t mqtt_unpack_suback_response(struct mqtt_response *mqtt_response, const uint8_t *buf);

/**
 * @brief Deserialize an UNSUBACK packet from \p buf.
 * @ingroup unpacker
 *  
 * @pre \ref mqtt_unpack_fixed_header must have returned a positive value and the mqtt_response must
 *      have a control type of \c MQTT_CONTROL_UNSUBACK.
 * 
 * @param[out] mqtt_response the response that is initialized from the contents of \p buf.
 * @param[in] buf the buffer with the incoming data.
 *
 * @relates mqtt_response_unsuback
 * 
 * @returns The number of bytes that were consumed, or 0 if the buffer does not contain enough 
 *          bytes to parse the packet, or a negative value if there was a protocol violation.
 */  
ssize_t mqtt_unpack_unsuback_response(struct mqtt_response *mqtt_response, const uint8_t *buf);

/**
 * @brief Deserialize a packet from the broker.
 * @ingroup unpackers
 * 
 * @param[out] response the mqtt_response that will be initialize from \p buf.
 * @param[in] buf the incoming data buffer.
 * @param[in] bufsz the number of bytes available in the buffer.
 * 
 * @relates mqtt_response
 * 
 * @returns The number of bytes consumed on success, zero \p buf does not contain enough bytes
 *          to deserialize the packet, a negative value if a protocol violation was encountered.  
 */
ssize_t mqtt_unpack_response(struct mqtt_response* response, const uint8_t *buf, size_t bufsz);

/* REQUESTS */

 /**
 * @brief Serialize an mqtt_fixed_header and write it to \p buf.
 * @ingroup packers
 * 
 * @note This function performs complete error checking and a positive return value
 *       guarantees the entire packet will fit into the given buffer.
 * 
 * @param[out] buf the buffer to write to.
 * @param[in] bufsz the maximum number of bytes that can be put in to \p buf.
 * @param[in] fixed_header the fixed header that will be serialized.
 * 
 * @returns The number of bytes written to \p buf, or 0 if \p buf is too small, or a 
 *          negative value if there was a protocol violation.
 */
ssize_t mqtt_pack_fixed_header(uint8_t *buf, size_t bufsz, const struct mqtt_fixed_header *fixed_header);

/**
 * @brief An enumeration of CONNECT packet flags.
 * @ingroup packers
 * 
 * @see <a href="http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718030">
 * MQTT v3.1.1: CONNECT Variable Header.
 * </a> 
 */
enum MQTTConnectFlags {
    MQTT_CONNECT_RESERVED = 1u,
    MQTT_CONNECT_CLEAN_SESSION = 2u,
    MQTT_CONNECT_WILL_FLAG = 4u,
    MQTT_CONNECT_WILL_QOS_0 = (0u & 0x03) << 3,
    MQTT_CONNECT_WILL_QOS_1 = (1u & 0x03) << 3,
    MQTT_CONNECT_WILL_QOS_2 = (2u & 0x03) << 3,
    MQTT_CONNECT_WILL_RETAIN = 32u,
    MQTT_CONNECT_PASSWORD = 64u,
    MQTT_CONNECT_USER_NAME = 128u,
};

/**
 * @brief Serialize a connection request into a buffer. 
 * @ingroup packers
 * 
 * @param[out] buf the buffer to pack the connection request packet into.
 * @param[in] bufsz the number of bytes left in \p buf.
 * @param[in] client_id the ID that identifies the local client. \p client_id is a required 
 *                      parameter.
 * @param[in] will_topic the topic under which the local client's will message will be published.
 *                       Set to \c NULL for no will message. If \p will_topic is not \c NULL a
 *                       \p will_message must also be provided.
 * @param[in] will_message the will message to be published upon a unsuccessful disconnection of
 *                         the local client. Set to \c NULL if \p will_topic is \c NULL. 
 *                         \p will_message must \em not be \c NULL if \p will_topic is not 
 *                         \c NULL.
 * @param[in] user_name the username to be used to connect to the broker with. Set to \c NULL if 
 *                      no username is required.
 * @param[in] password the password to be used to connect to the broker with. Set to \c NULL if
 *                     no password is required.
 * @param[in] connect_flags additional MQTTConnectFlags to be set. The only flags that need to be
 *                          set manually are \c MQTT_CONNECT_CLEAN_SESSION, 
 *                          \c MQTT_CONNECT_WILL_QOS_X (for \c X &isin; {0, 1, 2}), and 
 *                          \c MQTT_CONNECT_WILL_RETAIN. Set to 0 if no additional flags are 
 *                          required.
 * @param[in] keep_alive the keep alive time in seconds. It is the responsibility of the clinet 
 *                       to ensure packets are sent to the server \em {at least} this frequently.
 * 
 * @note If there is a \p will_topic and no additional \p connect_flags are given, then by 
 *       default \p will_message will be published at QoS level 0.
 * 
 * @see <a href="http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718028">
 * MQTT v3.1.1: CONNECT - Client Requests a Connection to a Server.
 * </a>
 * 
 * @returns The number of bytes put into \p buf, 0 if \p buf is too small to fit the CONNECT 
 *          packet, a negative value if there was a protocol violation.
 */
ssize_t mqtt_pack_connection_request(uint8_t* buf, size_t bufsz, 
                                     const char* client_id,
                                     const char* will_topic,
                                     const char* will_message,
                                     const char* user_name,
                                     const char* password,
                                     uint8_t connect_flags,
                                     uint16_t keep_alive);

/**
 * @brief An enumeration of the PUBLISH flags.
 * @ingroup packers
 * 
 * @see <a href="http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718037">
 * MQTT v3.1.1: PUBLISH - Publish Message.
 * </a>
 */
enum MQTTPublishFlags {
    MQTT_PUBLISH_DUP = 8u,
    MQTT_PUBLISH_QOS_0 = ((0u << 1) & 0x06),
    MQTT_PUBLISH_QOS_1 = ((1u << 1) & 0x06),
    MQTT_PUBLISH_QOS_2 = ((2u << 1) & 0x06),
    MQTT_PUBLISH_RETAIN = 0x01
};

/**
 * @brief Serialize a PUBLISH request and put it in \p buf.
 * @ingroup packers
 * 
 * @param[out] buf the buffer to put the PUBLISH packet in.
 * @param[in] bufsz the maximum number of bytes that can be put into \p buf.
 * @param[in] topic_name the topic to publish \p application_message under.
 * @param[in] packet_id this packets packet ID.
 * @param[in] application_message the application message to be published.
 * @param[in] application_message_size the size of \p application_message in bytes.
 * @param[in] publish_flags The flags to publish \p application_message with. These include
 *                          the \c MQTT_PUBLISH_DUP flag, \c MQTT_PUBLISH_QOS_X (\c X &isin; 
 *                          {0, 1, 2}), and \c MQTT_PUBLISH_RETAIN flag.
 * 
 * @note The default QoS is level 0.
 * 
 * @see <a href="http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718037">
 * MQTT v3.1.1: PUBLISH - Publish Message.
 * </a>
 * 
 * @returns The number of bytes put into \p buf, 0 if \p buf is too small to fit the PUBLISH 
 *          packet, a negative value if there was a protocol violation.
 */
ssize_t mqtt_pack_publish_request(uint8_t *buf, size_t bufsz,
                                  const char* topic_name,
                                  uint16_t packet_id,
                                  void* application_message,
                                  size_t application_message_size,
                                  uint8_t publish_flags);

/**
 * @brief Serialize a PUBACK, PUBREC, PUBREL, or PUBCOMP packet and put it in \p buf.
 * @ingroup packers
 * 
 * @param[out] buf the buffer to put the PUBXXX packet in.
 * @param[in] bufsz the maximum number of bytes that can be put into \p buf.
 * @param[in] control_type the type of packet. Must be one of: \c MQTT_CONTROL_PUBACK, 
 *                         \c MQTT_CONTROL_PUBREC, \c MQTT_CONTROL_PUBREL, 
 *                         or \c MQTT_CONTROL_PUBCOMP.
 * @param[in] packet_id the packet ID of the packet being acknowledged.
 * 
 * 
 * @see <a href="http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718043">
 * MQTT v3.1.1: PUBACK - Publish Acknowledgement.
 * </a>
 * @see <a href="http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718048">
 * MQTT v3.1.1: PUBREC - Publish Received.
 * </a>
 * @see <a href="http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718053">
 * MQTT v3.1.1: PUBREL - Publish Released.
 * </a>
 * @see <a href="http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718058">
 * MQTT v3.1.1: PUBCOMP - Publish Complete.
 * </a>
 * 
 * @returns The number of bytes put into \p buf, 0 if \p buf is too small to fit the PUBXXX 
 *          packet, a negative value if there was a protocol violation.
 */
ssize_t mqtt_pack_pubxxx_request(uint8_t *buf, size_t bufsz, 
                                 enum MQTTControlPacketType control_type,
                                 uint16_t packet_id);

/** 
 * @brief The maximum number topics that can be subscribed to in a single call to 
 *         mqtt_pack_subscribe_request.
 * @ingroup packers
 * 
 * @see mqtt_pack_subscribe_request
 */
#define MQTT_SUBSCRIBE_REQUEST_MAX_NUM_TOPICS 8

/** 
 * @brief Serialize a SUBSCRIBE packet and put it in \p buf.
 * @ingroup packers
 * 
 * @param[out] buf the buffer to put the SUBSCRIBE packet in.
 * @param[in] bufsz the maximum number of bytes that can be put into \p buf.
 * @param[in] packet_id the packet ID to be used.
 * @param[in] ... \c NULL terminated list of (\c {const char *topic_name}, \c {int max_qos_level})
 *                pairs.
 * 
 * @note The variadic arguments, \p ..., \em must be followed by a \c NULL. For example:
 * @code
 * ssize_t n = mqtt_pack_subscribe_request(buf, bufsz, 1234, "topic_1", 0, "topic_2", 2, NULL);
 * @endcode
 * 
 * @see <a href="http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718063">
 * MQTT v3.1.1: SUBSCRIBE - Subscribe to Topics.
 * </a>
 * 
 * @returns The number of bytes put into \p buf, 0 if \p buf is too small to fit the SUBSCRIBE 
 *          packet, a negative value if there was a protocol violation.
 */
ssize_t mqtt_pack_subscribe_request(uint8_t *buf, size_t bufsz, 
                                    uint16_t packet_id, 
                                    ...); /* null terminated */

/** 
 * @brief The maximum number topics that can be subscribed to in a single call to 
 *         mqtt_pack_unsubscribe_request.
 * @ingroup packers
 * 
 * @see mqtt_pack_unsubscribe_request
 */
#define MQTT_UNSUBSCRIBE_REQUEST_MAX_NUM_TOPICS 8

/** 
 * @brief Serialize a UNSUBSCRIBE packet and put it in \p buf.
 * @ingroup packers
 * 
 * @param[out] buf the buffer to put the UNSUBSCRIBE packet in.
 * @param[in] bufsz the maximum number of bytes that can be put into \p buf.
 * @param[in] packet_id the packet ID to be used.
 * @param[in] ... \c NULL terminated list of \c {const char *topic_name}'s to unsubscribe from.
 * 
 * @note The variadic arguments, \p ..., \em must be followed by a \c NULL. For example:
 * @code
 * ssize_t n = mqtt_pack_unsubscribe_request(buf, bufsz, 4321, "topic_1", "topic_2", NULL);
 * @endcode
 * 
 * @see <a href="http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718072">
 * MQTT v3.1.1: UNSUBSCRIBE - Unsubscribe from Topics.
 * </a>
 * 
 * @returns The number of bytes put into \p buf, 0 if \p buf is too small to fit the UNSUBSCRIBE 
 *          packet, a negative value if there was a protocol violation.
 */
ssize_t mqtt_pack_unsubscribe_request(uint8_t *buf, size_t bufsz, 
                                      uint16_t packet_id, 
                                      ...); /* null terminated */

/**
 * @brief Serialize a PINGREQ and put it into \p buf.
 * @ingroup packers
 * 
 * @param[out] buf the buffer to put the PINGREQ packet in.
 * @param[in] bufsz the maximum number of bytes that can be put into \p buf.
 * 
 * @see <a href="http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718081">
 * MQTT v3.1.1: PINGREQ - Ping Request.
 * </a>
 * 
 * @returns The number of bytes put into \p buf, 0 if \p buf is too small to fit the PINGREQ
 *          packet, a negative value if there was a protocol violation.
 */
ssize_t mqtt_pack_ping_request(uint8_t *buf, size_t bufsz);

/**
 * @brief Serialize a DISCONNECT and put it into \p buf.
 * @ingroup packers
 * 
 * @param[out] buf the buffer to put the DISCONNECT packet in.
 * @param[in] bufsz the maximum number of bytes that can be put into \p buf.
 * 
 * @see <a href="http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718090">
 * MQTT v3.1.1: DISCONNECT - Disconnect Notification.
 * </a>
 * 
 * @returns The number of bytes put into \p buf, 0 if \p buf is too small to fit the DISCONNECT 
 *          packet, a negative value if there was a protocol violation.
 */
ssize_t mqtt_pack_disconnect(uint8_t *buf, size_t bufsz);


/**
 * @brief An enumeration of queued message states. 
 */
enum MQTTQueuedMessageState {
    MQTT_QUEUED_UNSENT,
    MQTT_QUEUED_AWAITING_ACK,
    MQTT_QUEUED_COMPLETE
};

/**
 * @brief A message in a mqtt_message_queue.
 */
struct mqtt_queued_message {
    /** @brief A pointer to the start of the message. */
    uint8_t *start;

    /** @brief The number of bytes in the message. */
    size_t size;

    /** @brief The state of the message. */
    enum MQTTQueuedMessageState state;

    /** 
     * @brief The time at which the message was sent.. 
     * 
     * @note A timeout will only occur if the message is in
     *       the MQTT_QUEUED_AWAITING_ACK \c state.
     */
    time_t time_sent;

    /**
     * @brief The control type of the message.
     */
    enum MQTTControlPacketType control_type;

    /** 
     * @brief The packet id of the message.
     * 
     * @note This field is only used if the associate \c control_type has a 
     *       \c packet_id field.
     */
    uint16_t packet_id;
};

/**
 * @brief A message queue.
 * 
 * @note This struct is used internally to manage sending messages.
 * @note The only members the user should use are \c curr and \c curr_sz. 
 */
struct mqtt_message_queue {
    /** 
     * @brief The start of the message queue's memory block. 
     * 
     * @warning This member should \em not be manually changed.
     */
    void *mem_start;
    /** @brief The end of the message queue's memory block. */
    void *mem_end;

    /**
     * @brief A pointer to the position in the buffer you can pack bytes at.
     * 
     * @note Immediately after packing bytes at \c curr you \em must call
     *       mqtt_mq_register.
     */
    uint8_t *curr;

    /**
     * @brief The number of bytes that can be written to \c curr.
     * 
     * @note curr_sz will decrease by more than the number of bytes you write to 
     *       \c curr. This is because the mqtt_queued_message structs share the 
     *       same memory (and thus, a mqtt_queued_message must be allocated in 
     *       the message queue's memory whenever a new message is registered).  
     */
    size_t curr_sz;
    
    /**
     * @brief The tail of the array of mqtt_queued_messages's.
     * 
     * @note This member should not be used manually.
     */
    struct mqtt_queued_message *queue_tail;
};

/**
 * @brief Initialize a message queue.
 * 
 * @param[out] mq The message queue to initialize.
 * @param[in] buf The buffer for this message queue.
 * @param[in] bufsz The number of bytes in the buffer. 
 */
void mqtt_mq_init(struct mqtt_message_queue *mq, void *buf, size_t bufsz);

/**
 * @brief Clear as many messages from the front of the queue as possible.
 * 
 * @param mq The message queue.
 */
void mqtt_mq_clean(struct mqtt_message_queue *mq);

/**
 * @brief Register a message that was just added to the buffer.
 * 
 * @note This function should be called immediately following a call to a packer function
 *       that returned a positive value. The positive value (number of bytes packed) should
 *       be passed to this function.
 * 
 * @param mq The message queue.
 * @param[in] nbytes The number of bytes that were just packed.
 * 
 * @note This function will step mqtt_message_queue::curr and update mqtt_message_queue::curr_sz.
 * 
 * @returns The newly added struct mqtt_queued_message.
 */
struct mqtt_queued_message* mqtt_mq_register(struct mqtt_message_queue *mq, size_t nbytes);

struct mqtt_queued_message* mqtt_mq_find(struct mqtt_message_queue *mq, enum MQTTControlPacketType control_type, uint16_t *packet_id);

/**
 * @brief Returns the mqtt_queued_message at \p index.
 * 
 * @param mq_ptr A pointer to the message queue.
 * @param index The index of the message. 
 *
 * @returns The mqtt_queued_message at \p index.
 */
#define mqtt_mq_get(mq_ptr, index) (((struct mqtt_queued_message*) ((mq_ptr)->mem_end)) - 1 - index)

/**
 * @brief Returns the number of messages in the message queue, \p mq_ptr.
 */
#define mqtt_mq_length(mq_ptr) (((struct mqtt_queued_message*) ((mq_ptr)->mem_end)) - (mq_ptr)->queue_tail)

/**
 * @brief Used internally to recalculate the \c curr_sz.
 */
#define mqtt_mq_currsz(mq_ptr) (mq_ptr->curr >= (uint8_t*) ((mq_ptr)->queue_tail - 1)) ? 0 : ((uint8_t*) ((mq_ptr)->queue_tail - 1)) - (mq_ptr)->curr

#endif