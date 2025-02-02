#line 1 "D:\\AmplifyMobility\\Test COde\\Code- AC001-Shobha demo\\AC001_Three_Connectors\\src\\libraries\\arduinoWebSockets-master\\src\\libsha1\\libsha1.h"
/* ================ sha1.h ================ */
/*
SHA-1 in C
By Steve Reid <steve@edmweb.com>
100% Public Domain
*/

#if !defined(ESP8266) && !defined(ESP32)

typedef struct {
    uint32_t state[5];
    uint32_t count[2];
    unsigned char buffer[64];
} SHA1_CTX;

void SHA1Transform(uint32_t state[5], const unsigned char buffer[64]);
void SHA1Init(SHA1_CTX* context);
void SHA1Update(SHA1_CTX* context, const unsigned char* data, uint32_t len);
void SHA1Final(unsigned char digest[20], SHA1_CTX* context);

#endif
