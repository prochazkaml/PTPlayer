#include "ptplayer.h"
#include <math.h>
#include <string.h>

#define maxchannels 12
#define IBN 8
#define maxnote (3 + (8 * 12) + 1)
#define vibtablesize 32
#define vibtabledepth (IBN * 8)

songstatus_t s;

short notesHz[maxnote * IBN];
short VibTable[vibtablesize];

int samplesplayed;

int PTPlayer_Init(uint8_t *filedata) {
	int i, ch;
	
	if(filedata[0x5D] > maxchannels || memcmp(filedata + 1, "MONOTONE", 8)) {
		return 0;
	}

	memset(&s, 0, sizeof(songstatus_t));

	s.patterns = filedata[0x5C];
	s.channels = filedata[0x5D];

	for(s.orders = 0; s.orders < 256; s.orders++) {
		if(filedata[0x5F + s.orders] == 0xFF) break;

		s.ordertable[s.orders] = filedata[0x5F + s.orders];
	}

	double interval = pow(2, 1.0 / (12 * IBN));
	notesHz[0] = 440;
	double temphz = 27.5;
	notesHz[IBN] = round(temphz);

	for(i = IBN - 1; i >= 1; i--) {
		temphz /= interval;
		if(temphz < 19) temphz = 19;
		notesHz[i] = round(temphz);
	}

	temphz = 27.5;

	for(i = IBN + 1; i < maxnote * IBN; i++) {
		temphz *= interval;
		notesHz[i] = round(temphz);
	}

	s.tempo = (s.channels > 4) ? s.channels : 4;
	s.audiospeed = 60;

	for(ch = 0; ch < maxchannels; ch++) {
		s.channel[ch].enabled = 1;
		s.channel[ch].active = 0;
	}

	for(i = 0; i < vibtablesize; i++) {
		VibTable[i] = round(vibtabledepth * sin(((double)i) * M_PI / vibtablesize * 2));
	}

	s.data = (uint16_t *)(filedata + 0x15F);

	s.row = 0x3F;
	s.order = -1;
	s.tempotick = 1;
	s.audiotick = 1;

	samplesplayed = 0;

	return 1;
}

songstatus_t *PTPlayer_GetStatus() {
	return &s;
}

void PTPlayer_ProcessTick() {
	int i, ch;

	if(!--s.tempotick) {
		// Process a new row

		if(++s.row >= 0x40) {
			// Process the next order

			s.order++;
			s.row = 0;
		}

		for(ch = 0; ch < s.channels; ch++) {
			switch(s.channel[ch].effect) {
				case 5:
					s.order = s.channel[ch].parm1;
					s.row = 0;
					break;

				case 6:
					s.order++;
					s.row = s.channel[ch].parm1;
					break;
			}
		}

		if(s.order >= s.orders) s.order = 0;

		s.rowdata = s.data + (s.ordertable[s.order] * 64 + s.row) * s.channels;

		for(ch = 0; ch < s.channels; ch++) {
			s.channel[ch].note = s.rowdata[ch] >> 9;
			s.channel[ch].effectraw = s.rowdata[ch] & 0x1FF;
			s.channel[ch].effect = (s.rowdata[ch] >> 6) & 7;

			if(s.channel[ch].effect == 0 || s.channel[ch].effect == 4) {
				s.channel[ch].parm1 = (s.rowdata[ch] >> 3) & 7;
				s.channel[ch].parm2 = s.rowdata[ch] & 7;
			} else {
				s.channel[ch].parm1 = s.rowdata[ch] & 63;
				s.channel[ch].parm2 = 0;
			}

			if(s.channel[ch].note == 127) {
				s.channel[ch].active = 0;
			} else if(s.channel[ch].note && s.channel[ch].note <= maxnote && s.channel[ch].effect != 3) {
				s.channel[ch].active = 1;
				s.channel[ch].interval = s.channel[ch].note * IBN;
				s.channel[ch].freq = notesHz[s.channel[ch].interval];
				s.channel[ch].lastnote = s.channel[ch].note;
				s.channel[ch].vibindex = 0;
				s.channel[ch].ctr = 0;
			}

			if(s.channel[ch].effect) switch(s.channel[ch].effect) {
				case 3:
					if(s.channel[ch].note && s.channel[ch].note <= maxnote)
						s.channel[ch].portafreq = notesHz[s.channel[ch].note * IBN];
					
					if(s.channel[ch].parm1) s.channel[ch].portadelta = s.channel[ch].parm1;
					break;

				case 4:
					if(s.channel[ch].parm1) s.channel[ch].vibspeed = s.channel[ch].parm1;
					if(s.channel[ch].parm2) s.channel[ch].vibdepth = s.channel[ch].parm2;
					s.channel[ch].vibindex += s.channel[ch].vibspeed;
					s.channel[ch].vibindex %= vibtablesize;
					break;

				case 7:
					if(s.channel[ch].parm1 > 0x20) {
						s.audiospeed = s.channel[ch].parm1;
					} else {
						s.tempo = s.channel[ch].parm1;
					}

					break;
			}
		}

		s.tempotick = s.tempo;
	} else {
		// Process effects

		for(ch = 0; ch < s.channels; ch++) {
			if(s.channel[ch].effectraw) {
				switch(s.channel[ch].effect) {
					case 0:
						switch((s.tempo - s.tempotick) % 3) {
							case 0:
								s.channel[ch].interval = s.channel[ch].lastnote * IBN;
								break;

							case 1:
								s.channel[ch].interval = (s.channel[ch].lastnote + s.channel[ch].parm1) * IBN;
								break;

							case 2:
								s.channel[ch].interval = (s.channel[ch].lastnote + s.channel[ch].parm2) * IBN;
								break;
						}

						s.channel[ch].freq = notesHz[s.channel[ch].interval];
						break;

					case 1:
						s.channel[ch].freq += s.channel[ch].parm1;
						// I mean, it's not _really_ necessary to clamp the upper frequency limit
						break;

					case 2:
						s.channel[ch].freq -= s.channel[ch].parm1;
						if(s.channel[ch].freq < 20) s.channel[ch].freq = 20;
						break;

					case 3:
						if(s.channel[ch].freq < s.channel[ch].portafreq) {
							s.channel[ch].freq += s.channel[ch].portadelta;

							if(s.channel[ch].freq > s.channel[ch].portafreq) {
								s.channel[ch].freq = s.channel[ch].portafreq;
							}
						}
						
						if(s.channel[ch].freq > s.channel[ch].portafreq) {
							s.channel[ch].freq -= s.channel[ch].portadelta;

							if(s.channel[ch].freq < s.channel[ch].portafreq) {
								s.channel[ch].freq = s.channel[ch].portafreq;
							}
						}
						break;

					case 4:
						s.channel[ch].freq = notesHz[s.channel[ch].interval + (((VibTable[s.channel[ch].vibindex]) * s.channel[ch].vibdepth) / vibtabledepth)];
						s.channel[ch].vibindex += s.channel[ch].vibspeed;
						s.channel[ch].vibindex %= vibtablesize;
						break;
				}
			}
		}
	}
}

int PTPlayer_PlayInt16(int16_t *buf, int bufsize, int audiofreq) {
	int i, ch;

	memset(buf, 0, bufsize * sizeof(short));

	for(i = 0; i < bufsize; i++) {
		// Process tick if necessary

		if(!--s.audiotick) {
			PTPlayer_ProcessTick();

			s.audiotick = audiofreq / s.audiospeed;
		}

		// Render the sound

		for(ch = 0; ch < s.channels; ch++) {
			if(s.channel[ch].active && s.channel[ch].enabled) {
				s.channel[ch].ctr += s.channel[ch].freq;

				if((s.channel[ch].ctr / (audiofreq / 2)) & 1) {
					buf[i] += 32767 / s.channels;
				}
			}
		}
	}

	return samplesplayed += bufsize;
}

int PTPlayer_PlayFloat(float *buf, int bufsize, int audiofreq) {
	int i, ch;

	memset(buf, 0, bufsize * sizeof(float));

	for(i = 0; i < bufsize; i++) {
		// Process tick if necessary

		if(!--s.audiotick) {
			PTPlayer_ProcessTick();

			s.audiotick = audiofreq / s.audiospeed;
		}

		// Render the sound

		for(ch = 0; ch < s.channels; ch++) {
			if(s.channel[ch].active && s.channel[ch].enabled) {
				s.channel[ch].ctr += s.channel[ch].freq;

				if((s.channel[ch].ctr / (audiofreq / 2)) & 1) {
					buf[i] += 1.0 / ((float)s.channels);
				}
			}
		}
	}

	return samplesplayed += bufsize;
}

int notectr = 0;

void PTPlayer_PlayNoteInt16(uint8_t note, int16_t *buf, int bufsize, int audiofreq) {
	int i;

	memset(buf, 0, bufsize * sizeof(int16_t));

	for(i = 0; i < bufsize; i++) {
		// Render the sound

		notectr += notesHz[note * IBN];

		if((notectr / (audiofreq / 2)) & 1) {
			buf[i] += 32767 / s.channels;
		}
	}
}

void PTPlayer_PlayNoteFloat(uint8_t note, float *buf, int bufsize, int audiofreq) {
	int i;

	memset(buf, 0, bufsize * sizeof(float));

	for(i = 0; i < bufsize; i++) {
		// Render the sound

		notectr += notesHz[note * IBN];

		if((notectr / (audiofreq / 2)) & 1) {
			buf[i] += 1.0 / ((float)s.channels);
		}
	}

}
