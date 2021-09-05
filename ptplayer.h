#include <stdint.h>

typedef struct {
	int16_t interval, freq, portafreq, effectraw;
	uint8_t enabled, active;

	uint8_t note, effect, parm1, parm2;
	uint8_t lastnote, portadelta;
	uint8_t vibspeed, vibdepth, vibindex;

	int ctr;
} channel_t;

typedef struct {
	uint16_t *data, *rowdata;

	uint8_t patterns, channels, ordertable[256];
	int row, order, orders, tempo, tempotick, audiospeed, audiotick;

	channel_t channel[12];
} songstatus_t;

int PTPlayer_Init(uint8_t *filedata);
songstatus_t *PTPlayer_GetStatus();
void PTPlayer_ProcessTick();
int PTPlayer_PlayInt16(int16_t *buf, int bufsize, int audiofreq);
int PTPlayer_PlayFloat(float *buf, int bufsize, int audiofreq);

void PTPlayer_PlayNoteInt16(uint8_t note, int16_t *buf, int bufsize, int audiofreq);
void PTPlayer_PlayNoteFloat(uint8_t note, float *buf, int bufsize, int audiofreq);
