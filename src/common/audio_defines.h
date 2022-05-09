#ifndef _AUDIO_DEFINES_H_
#define _AUDIO_DEFINES_H_


#define LIMITER_MaximumTruePeak     -1.0f
#define LIMITER_AttackSec           0.001f
#define LIMITER_ReleaseSec          0.200f
#define LIMITER_LookAhead           240

typedef enum  {
  CHANNELUNKNOWN = 0,
  CHANNELMONO = 1, // 1
  CHANNELSTEREO = 2, // 2
  CHANNEL51 = 6, // 6
  CHANNEL71 = 8, // 8
  CHANNEL312 = 3, // 3
  CHANNEL512 = 5, // 5
  CHANNEL714 = 7 // 7
}channelLayout;

#define MAX_CHANNELS 12
#define MAX_DELAYSIZE 4096
#define CHUNK_SIZE 960
#define FRAME_SIZE 960
#define MAX_PACKET_SIZE  (MAX_CHANNELS*sizeof(int16_t)*FRAME_SIZE) // 960*2/channel


#endif
