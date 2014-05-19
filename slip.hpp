//////////////////////////////////////////////////////////////////////
// slip.hpp -- SLIP Protocol Class
// Date: Fri May 16 19:54:36 2014  Warren Gay ve3wwg
///////////////////////////////////////////////////////////////////////
//
// SLIP protocol object :
//
// This is useful for "framing" serial binary messages between the
// local host processor and the remote host.
//
// I/O is performed through user provided read and write callbacks.
//
// NOTES:
//
//  1.  This implements the standard uncompressed SLIP by default. 
//  2.  A non-SLIP standard 8-bit CRC can be automatically appended
//      to each packet written when enabled by enable_crc8(true).
//      Each packet received will have the last byte checked (not returned)
//      to see if it matches the calculated CRC. A failed status of
//      ES_CRC is returned if it does not match.
//  3.  This particular software copy is intended to be used by POSIX systems
//      communicating with AVR/Teensy systems (over a serial connection,
//      which may be Bluetooth).

#ifndef SLIP_HPP
#define SLIP_HPP

#include <stdint.h>

extern "C" {
	typedef uint8_t (*slipread_t)();        // Read byte user routine
	typedef void (*slipwrite_t)(uint8_t b); // Write byte user routine
}

class SLIP {
	slipread_t		read_b;		// Routine to fetch data with
	slipwrite_t		write_b;	// Routine to write data with
	bool			use_crc8;	// Include CRC8 in packet

	void write_encoded(uint8_t b);		// Write encoded byte

public:	enum Status {
		ES_Ok = 0,			// Success
		ES_Trunc,			// Packet was too long for buffer (truncated)
		ES_Protocol,			// Protocol error
		ES_CRC				// CRC Failure
	};

	SLIP(slipread_t read_func,slipwrite_t write_func);
	bool enable_crc8(bool on);
	void write(const void *buffer,unsigned length);
	Status read(void *buffer,unsigned buflen,unsigned& retlength);
};

#endif // SLIP_HPP

// End slip.hpp
