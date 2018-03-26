#ifndef __MQTT_RESPONSES_H__
#define __MQTT_RESPONSES_H__

#include <stdint.h>     //< uintXX_t ...
#include <stddef.h>     //< size_t
#include <sys/types.h>  //< ssize_t

/**
 * @file 
 */

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
    size_t appilcation_message_size;
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



#endif