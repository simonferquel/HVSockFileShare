#pragma once
#define __cdecl
#include <cstdint>

/* GUID handling  */
#ifdef _WIN32
#include <initguid.h>
#else
typedef struct _GUID {
	uint32_t Data1;
	uint16_t Data2;
	uint16_t Data3;
	uint8_t  Data4[8];
} GUID;

#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
const GUID name \
= { l, w1, w2,{ b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
#endif
DEFINE_GUID(HV_GUID_ZERO, 0x00000000, 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);


/* HV Socket definitions */
#ifdef _WIN32
#define AF_HYPERV 34
#else
#define AF_HYPERV ((AF_MAX>43)? 43 : AF_X25)
#endif

#define HV_PROTOCOL_RAW 1


#pragma pack(push,1)
typedef struct _SOCKADDR_HV
{
	unsigned short Family;
	unsigned short Reserved;
	GUID VmId;
	GUID ServiceId;
} SOCKADDR_HV;
#pragma pack(pop)



