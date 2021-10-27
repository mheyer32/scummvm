/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "backends/mixer/amigaos3/amigaos3-mixer.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/system.h"
#include "common/textconsole.h"

// Amiga includes.
#include <dos/dostags.h>
#include <proto/ahi.h>
#include <proto/dos.h>
#include <proto/exec.h>

#define SAMPLES_PER_SEC 11025

static struct MsgPort *ahiPort = NULL;

static struct AHIRequest *ahiReq[2] = {NULL, NULL};
static bool ahiReqSent[2] = {false, FALSE};

static BYTE *soundBuffer[2] = {NULL, NULL};
static BYTE _currentSoundBuffer = 0;

static uint32 _mixingFrequency = 0;
static uint32 _sampleCount = 0;
static uint32 _sampleBufferSize = 0;

static Audio::MixerImpl *g_mixer = NULL;
static struct Task *g_soundThread = NULL;
static struct Task *g_mainThread = NULL;

AmigaOS3MixerManager::AmigaOS3MixerManager() {
#ifndef NDEBUG
	debug(9, "AmigaOS3MixerManager::AmigaOS3MixerManager()");
#endif
}

AmigaOS3MixerManager::~AmigaOS3MixerManager() {
#ifndef NDEBUG
	debug(9, "AmigaOS3MixerManager::~AmigaOS3MixerManager()");
#endif

	if (g_mixer) {
		g_mixer->setReady(false);

		if (g_soundThread) {
			Signal(g_soundThread, SIGBREAKF_CTRL_C);
			Wait(SIGBREAKF_CTRL_F);
			g_soundThread = NULL;
		}

		delete g_mixer;
		g_mixer = NULL;
	}
}

void exit_scummvm_sound() {
	if (ahiReq[1]) {
		FreeVec(ahiReq[1]);
	}

	if (ahiReq[0]) {
		CloseDevice((struct IORequest *)ahiReq[0]);
		DeleteIORequest(ahiReq[0]);
	}

	if (soundBuffer[0])
		FreeVec((APTR)soundBuffer[0]);

	if (soundBuffer[1])
		FreeVec((APTR)soundBuffer[1]);

	if (ahiPort)
		DeleteMsgPort(ahiPort);
}

#define SOUND_CHECK_FAILED \
	exit_scummvm_sound(); \
	return false;

bool check_scummvm_sound() {
	ahiPort = (struct MsgPort *)CreateMsgPort();
	if (!ahiPort) {
		error("check_scummvm_sound: Could not create a Message Port for AHI");
		SOUND_CHECK_FAILED
	}

	ahiReq[0] = (struct AHIRequest *)CreateIORequest(ahiPort, sizeof(struct AHIRequest));
	if (!ahiReq[0]) {
		error("check_scummvm_sound: Could not create an IO Request for AHI");
		SOUND_CHECK_FAILED
	}

	ahiReq[0]->ahir_Version = 4;
	BYTE deviceError = OpenDevice(AHINAME, AHI_DEFAULT_UNIT, (struct IORequest *)ahiReq[0], 0);
	if (deviceError) {
		error("check_scummvm_sound: Unable to open AHI Device");
		SOUND_CHECK_FAILED
	}

	return true;
}

bool init_scummvm_sound() {
	ahiPort = (struct MsgPort *)CreateMsgPort();
	if (!ahiPort) {
		error("Could not create a Message Port for AHI");
		return false;
	}

	ahiReq[0] = (struct AHIRequest *)CreateIORequest(ahiPort, sizeof(struct AHIRequest));
	if (!ahiReq[0]) {
		error("Could not create an IO Request for AHI");
		return false;
	}

	// Open at least version 4.
	ahiReq[0]->ahir_Version = 4;

	BYTE deviceError = OpenDevice(AHINAME, AHI_DEFAULT_UNIT, (struct IORequest *)ahiReq[0], 0);
	if (deviceError) {
		error("Unable to open AHI Device");
		return false;
	}

	// 32 bits (4 bytes) are required per sample for storage (16bit stereo).
	_sampleBufferSize = (_sampleCount * 4);

	soundBuffer[0] = (BYTE *)AllocVec(_sampleBufferSize, MEMF_PUBLIC | MEMF_CLEAR);
	soundBuffer[1] = (BYTE *)AllocVec(_sampleBufferSize, MEMF_PUBLIC | MEMF_CLEAR);

	if (!soundBuffer[0] || !soundBuffer[1]) {
		error("Could not create soundbuffers for AHI");
		return false;
	}
	// Make a copy of the request (for double buffering)
	ahiReq[1] = (struct AHIRequest *)AllocVec(sizeof(struct AHIRequest), MEMF_PUBLIC);
	if (!ahiReq[1]) {
		error("Could not create IO request copy for AHI");
		return false;
	}

	CopyMem(ahiReq[0], ahiReq[1], sizeof(struct AHIRequest));

	_currentSoundBuffer = 0;
	ahiReqSent[0] = false;
	ahiReqSent[1] = false;

	return true;
}

void __stdargs __saveds scummvm_sound_thread() {
	// int __saveds scummvm_sound_thread(void) {
	LONG priority = 0;
	ULONG signals;

	init_scummvm_sound();

	for (;;) {
		while (!ahiReqSent[_currentSoundBuffer] || CheckIO((IORequest *)ahiReq[_currentSoundBuffer])) {
			if (ahiReqSent[_currentSoundBuffer]) {
				WaitIO((IORequest *)ahiReq[_currentSoundBuffer]);
			}

			ahiReq[_currentSoundBuffer]->ahir_Std.io_Message.mn_Node.ln_Pri = priority;
			ahiReq[_currentSoundBuffer]->ahir_Std.io_Command = CMD_WRITE;
			ahiReq[_currentSoundBuffer]->ahir_Std.io_Data = soundBuffer[_currentSoundBuffer];
			ahiReq[_currentSoundBuffer]->ahir_Std.io_Offset = 0;
			ahiReq[_currentSoundBuffer]->ahir_Type = AHIST_S16S;
			ahiReq[_currentSoundBuffer]->ahir_Frequency = _mixingFrequency;
			ahiReq[_currentSoundBuffer]->ahir_Position = 0x8000;
			ahiReq[_currentSoundBuffer]->ahir_Volume = 0x10000;
			ahiReq[_currentSoundBuffer]->ahir_Link =
			  (ahiReqSent[_currentSoundBuffer ^ 1]) ? ahiReq[_currentSoundBuffer ^ 1] : NULL;

//			ahiReq[_currentSoundBuffer]->ahir_Std.io_Length = _sampleBufferSize;

			int mixedSamples = g_mixer->mixCallback((byte *)soundBuffer[_currentSoundBuffer], _sampleBufferSize);
			if (mixedSamples) {
				ahiReq[_currentSoundBuffer]->ahir_Std.io_Length = mixedSamples * 4;
				SendIO((struct IORequest *)ahiReq[_currentSoundBuffer]);
				ahiReqSent[_currentSoundBuffer] = true;
				// Flip.
				_currentSoundBuffer ^= 1;
			} else {
				// since we're not produceing any sound here, add a sleep for the required period??
				ahiReqSent[_currentSoundBuffer] = false;
				Delay(1);

				signals = SetSignal(0L, 0L);
				if (signals & SIGBREAKF_CTRL_C) {
					goto exit_thread;
				}
			}
		}
		// We're playing a sound right now, wait for it to finish
		signals = Wait(SIGBREAKF_CTRL_C | (1 << ahiPort->mp_SigBit));
		if (signals & SIGBREAKF_CTRL_C) {
			break;
		}
	}

exit_thread:

	if (ahiReqSent[_currentSoundBuffer]) {
		AbortIO((struct IORequest *)ahiReq[_currentSoundBuffer]);
		WaitIO((struct IORequest *)ahiReq[_currentSoundBuffer]);
	}

	if (ahiReqSent[_currentSoundBuffer ^ 1]) {
		AbortIO((struct IORequest *)ahiReq[!_currentSoundBuffer]);
		WaitIO((struct IORequest *)ahiReq[!_currentSoundBuffer]);
	}

	exit_scummvm_sound();
	Signal(g_mainThread, SIGBREAKF_CTRL_F);
}

void AmigaOS3MixerManager::init(int priority) {
#ifndef NDEBUG
	debug(9, "AmigaOS3MixerManager::init()");
#endif

	// Determine the desired output sampling frequency.
	if (ConfMan.hasKey("output_rate")) {
		_mixingFrequency = ConfMan.getInt("output_rate");
	}

	if (_mixingFrequency == 0) {
		_mixingFrequency = SAMPLES_PER_SEC;
	}

	// Determine the sample buffer size. We want it to store enough data for
	// at least 1/16th of a second (though at most 8192 samples). Note
	// that it must be a power of two. So e.g. at 22050 Hz, we request a
	// sample buffer size of 2048.
	switch(_mixingFrequency) {
		case 4000:
			_sampleCount = 1024;
			break;
		case 8000:
		case 11025:
			_sampleCount = 2048;
			break;
		default:
		case 22050:
			_sampleCount = 4096;
			break;
		case 44100:
		case 48000:
		case 96000:
			_sampleCount = 8192;
			break;
	}

	/*while ((_sampleCount * 4) > (_mixingFrequency * 2)) {
		_sampleCount >>= 1;
	}*/

	if (!check_scummvm_sound())
		return;

	// Create the mixer instance and start the sound processing.
	assert(!g_mixer);
	g_mixer = new Audio::MixerImpl(_mixingFrequency);
	assert(g_mixer);
	g_mixer->setReady(true);

	g_mainThread=FindTask(NULL);

	g_soundThread =
	  (Task *)CreateNewProcTags(NP_Name, (ULONG) "ScummVM MixerThread", NP_CloseOutput, FALSE, NP_CloseInput, FALSE,
								NP_StackSize, 20000, NP_Entry, (ULONG)&scummvm_sound_thread, TAG_DONE);

	if (!g_soundThread) {
		error("Could not create the sound thread");
	}
	else {
#ifndef NDEBUG
		debug(1, "Setting audio thread priority to %d", priority);
#endif
		SetTaskPri(g_soundThread, priority);
	}
}

Audio::Mixer *AmigaOS3MixerManager::getMixer() {
	return g_mixer;
}
