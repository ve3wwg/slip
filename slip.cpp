//////////////////////////////////////////////////////////////////////
// slip.cpp -- SLIP Class Implementation
// Date: Fri May 16 20:02:49 2014  Warren W. Gay
///////////////////////////////////////////////////////////////////////

#include "slip.hpp"
#include "crc8.cpp"

#define END             0300                    // indicates end of packet
#define ESC             0333                    // indicates byte stuffing
#define ESC_END         0334                    // ESC ESC_END means END data byte
#define ESC_ESC         0335                    // ESC ESC_ESC means ESC data byte

//////////////////////////////////////////////////////////////////////
// SLIP constructor :
//////////////////////////////////////////////////////////////////////

SLIP::SLIP(slipread_t read_func,slipwrite_t write_func) {
        read_b 	= read_func;    	// Byte reader
        write_b	= write_func;   	// Byte writer
	use_crc8 = false;
}

//////////////////////////////////////////////////////////////////////
// Enable I/O with CRC8
//////////////////////////////////////////////////////////////////////

bool
SLIP::enable_crc8(bool on) {
	bool r = use_crc8;

	use_crc8 = on;
	return r;
}

//////////////////////////////////////////////////////////////////////
// Write a packet using the user supplied write routine.
// This writes a packet from buffer of length bytes.
//////////////////////////////////////////////////////////////////////

void
SLIP::write(const void *buffer,unsigned length) {
	const unsigned msglen = length;
	const uint8_t *buf = (uint8_t *)buffer;
        uint8_t b;

        write_b(END);				// Flush out any line noise 

        while ( length-- > 0 ) {
                b = *buf++;                  	// Get next byte

		write_encoded(b);
        }

	if ( use_crc8 ) {
		uint8_t crc = crc8((uint8_t*)buffer,msglen);
		write_encoded(crc);		// Send CRC as last byte
	}

	write_b(END);				// Mark the end of the packet
}

//////////////////////////////////////////////////////////////////////
// Read a packet using the user supplied read routine.
// This reads a packet into the buffer with max length
// of buflen bytes.
//
// RETURNS :
//      ES_Ok      	Success (length of returned packet)
//      ES_Trunc     	Packet too long (truncated)
//      ES_Protocol  	Protocol error (bad ESC ch)
//	ES_CRC		CRC check failed
//////////////////////////////////////////////////////////////////////

SLIP::Status
SLIP::read(void *buffer,unsigned buflen,unsigned& retlength) {
	uint8_t *buf = (uint8_t *)buffer;
	uint8_t b;

	retlength = 0;

	for (;;) {
		b = read_b();

                switch ( b ) {
                case END :
                        if ( !retlength )
                                continue;               // Just ignore this initial END control byte

			if ( use_crc8 ) {
				if ( retlength <= 1 )
					return ES_CRC;	// Packet consisted only of CRC byte

				--retlength;		// Last byte is CRC8
				uint8_t recv_crc = buf[retlength];
				uint8_t calc_crc = crc8(buf,retlength);
				if ( recv_crc != calc_crc )
					return ES_CRC;
			}
			return ES_Ok;
                case ESC :
			b = read_b();

                        switch ( b ) {
                        case ESC_END :
                                b = END;
                                break;
                        case ESC_ESC :
                                b = ESC;
                                break;
                        default :
                                return ES_Protocol;	// Protocol error occurred
                        }
                }

                if ( retlength >= buflen )
                        return ES_Trunc;		// Packet too long for buffer
                buf[retlength++] = b;			// Receive this data byte
	}
}

//////////////////////////////////////////////////////////////////////
// Internal: Write encoded 
//////////////////////////////////////////////////////////////////////

void
SLIP::write_encoded(uint8_t b) {

	switch ( b ) {
	case END :
		write_b(ESC);
		write_b(ESC_END);
		break;
	case ESC :
		write_b(ESC);
		write_b(ESC_ESC);
		break;
	default :
		write_b(b);
	}
}

// End slip.cpp
