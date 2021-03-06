// MESSAGE LLC_OUT PACKING

#define MAVLINK_MSG_ID_LLC_OUT 186

typedef struct __mavlink_llc_out_t
{
 int16_t servoOut[4]; ///< 
                
            
 int16_t MotorOut[2]; ///< 
                
            
} mavlink_llc_out_t;

#define MAVLINK_MSG_ID_LLC_OUT_LEN 12
#define MAVLINK_MSG_ID_186_LEN 12

#define MAVLINK_MSG_ID_LLC_OUT_CRC 5
#define MAVLINK_MSG_ID_186_CRC 5

#define MAVLINK_MSG_LLC_OUT_FIELD_SERVOOUT_LEN 4
#define MAVLINK_MSG_LLC_OUT_FIELD_MOTOROUT_LEN 2

#define MAVLINK_MESSAGE_INFO_LLC_OUT { \
	"LLC_OUT", \
	2, \
	{  { "servoOut", NULL, MAVLINK_TYPE_INT16_T, 4, 0, offsetof(mavlink_llc_out_t, servoOut) }, \
         { "MotorOut", NULL, MAVLINK_TYPE_INT16_T, 2, 8, offsetof(mavlink_llc_out_t, MotorOut) }, \
         } \
}


/**
 * @brief Pack a llc_out message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param servoOut 
                
            
 * @param MotorOut 
                
            
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_llc_out_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
						       const int16_t *servoOut, const int16_t *MotorOut)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[MAVLINK_MSG_ID_LLC_OUT_LEN];

	_mav_put_int16_t_array(buf, 0, servoOut, 4);
	_mav_put_int16_t_array(buf, 8, MotorOut, 2);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_LLC_OUT_LEN);
#else
	mavlink_llc_out_t packet;

	mav_array_memcpy(packet.servoOut, servoOut, sizeof(int16_t)*4);
	mav_array_memcpy(packet.MotorOut, MotorOut, sizeof(int16_t)*2);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_LLC_OUT_LEN);
#endif

	msg->msgid = MAVLINK_MSG_ID_LLC_OUT;
#if MAVLINK_CRC_EXTRA
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_LLC_OUT_LEN, MAVLINK_MSG_ID_LLC_OUT_CRC);
#else
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_LLC_OUT_LEN);
#endif
}

/**
 * @brief Pack a llc_out message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param servoOut 
                
            
 * @param MotorOut 
                
            
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_llc_out_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
							   mavlink_message_t* msg,
						           const int16_t *servoOut,const int16_t *MotorOut)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[MAVLINK_MSG_ID_LLC_OUT_LEN];

	_mav_put_int16_t_array(buf, 0, servoOut, 4);
	_mav_put_int16_t_array(buf, 8, MotorOut, 2);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_LLC_OUT_LEN);
#else
	mavlink_llc_out_t packet;

	mav_array_memcpy(packet.servoOut, servoOut, sizeof(int16_t)*4);
	mav_array_memcpy(packet.MotorOut, MotorOut, sizeof(int16_t)*2);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_LLC_OUT_LEN);
#endif

	msg->msgid = MAVLINK_MSG_ID_LLC_OUT;
#if MAVLINK_CRC_EXTRA
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_LLC_OUT_LEN, MAVLINK_MSG_ID_LLC_OUT_CRC);
#else
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_LLC_OUT_LEN);
#endif
}

/**
 * @brief Encode a llc_out struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param llc_out C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_llc_out_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_llc_out_t* llc_out)
{
	return mavlink_msg_llc_out_pack(system_id, component_id, msg, llc_out->servoOut, llc_out->MotorOut);
}

/**
 * @brief Encode a llc_out struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param llc_out C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_llc_out_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_llc_out_t* llc_out)
{
	return mavlink_msg_llc_out_pack_chan(system_id, component_id, chan, msg, llc_out->servoOut, llc_out->MotorOut);
}

/**
 * @brief Send a llc_out message
 * @param chan MAVLink channel to send the message
 *
 * @param servoOut 
                
            
 * @param MotorOut 
                
            
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_llc_out_send(mavlink_channel_t chan, const int16_t *servoOut, const int16_t *MotorOut)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[MAVLINK_MSG_ID_LLC_OUT_LEN];

	_mav_put_int16_t_array(buf, 0, servoOut, 4);
	_mav_put_int16_t_array(buf, 8, MotorOut, 2);
#if MAVLINK_CRC_EXTRA
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_LLC_OUT, buf, MAVLINK_MSG_ID_LLC_OUT_LEN, MAVLINK_MSG_ID_LLC_OUT_CRC);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_LLC_OUT, buf, MAVLINK_MSG_ID_LLC_OUT_LEN);
#endif
#else
	mavlink_llc_out_t packet;

	mav_array_memcpy(packet.servoOut, servoOut, sizeof(int16_t)*4);
	mav_array_memcpy(packet.MotorOut, MotorOut, sizeof(int16_t)*2);
#if MAVLINK_CRC_EXTRA
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_LLC_OUT, (const char *)&packet, MAVLINK_MSG_ID_LLC_OUT_LEN, MAVLINK_MSG_ID_LLC_OUT_CRC);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_LLC_OUT, (const char *)&packet, MAVLINK_MSG_ID_LLC_OUT_LEN);
#endif
#endif
}

#if MAVLINK_MSG_ID_LLC_OUT_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This varient of _send() can be used to save stack space by re-using
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_llc_out_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  const int16_t *servoOut, const int16_t *MotorOut)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char *buf = (char *)msgbuf;

	_mav_put_int16_t_array(buf, 0, servoOut, 4);
	_mav_put_int16_t_array(buf, 8, MotorOut, 2);
#if MAVLINK_CRC_EXTRA
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_LLC_OUT, buf, MAVLINK_MSG_ID_LLC_OUT_LEN, MAVLINK_MSG_ID_LLC_OUT_CRC);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_LLC_OUT, buf, MAVLINK_MSG_ID_LLC_OUT_LEN);
#endif
#else
	mavlink_llc_out_t *packet = (mavlink_llc_out_t *)msgbuf;

	mav_array_memcpy(packet->servoOut, servoOut, sizeof(int16_t)*4);
	mav_array_memcpy(packet->MotorOut, MotorOut, sizeof(int16_t)*2);
#if MAVLINK_CRC_EXTRA
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_LLC_OUT, (const char *)packet, MAVLINK_MSG_ID_LLC_OUT_LEN, MAVLINK_MSG_ID_LLC_OUT_CRC);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_LLC_OUT, (const char *)packet, MAVLINK_MSG_ID_LLC_OUT_LEN);
#endif
#endif
}
#endif

#endif

// MESSAGE LLC_OUT UNPACKING


/**
 * @brief Get field servoOut from llc_out message
 *
 * @return 
                
            
 */
static inline uint16_t mavlink_msg_llc_out_get_servoOut(const mavlink_message_t* msg, int16_t *servoOut)
{
	return _MAV_RETURN_int16_t_array(msg, servoOut, 4,  0);
}

/**
 * @brief Get field MotorOut from llc_out message
 *
 * @return 
                
            
 */
static inline uint16_t mavlink_msg_llc_out_get_MotorOut(const mavlink_message_t* msg, int16_t *MotorOut)
{
	return _MAV_RETURN_int16_t_array(msg, MotorOut, 2,  8);
}

/**
 * @brief Decode a llc_out message into a struct
 *
 * @param msg The message to decode
 * @param llc_out C-struct to decode the message contents into
 */
static inline void mavlink_msg_llc_out_decode(const mavlink_message_t* msg, mavlink_llc_out_t* llc_out)
{
#if MAVLINK_NEED_BYTE_SWAP
	mavlink_msg_llc_out_get_servoOut(msg, llc_out->servoOut);
	mavlink_msg_llc_out_get_MotorOut(msg, llc_out->MotorOut);
#else
	memcpy(llc_out, _MAV_PAYLOAD(msg), MAVLINK_MSG_ID_LLC_OUT_LEN);
#endif
}
