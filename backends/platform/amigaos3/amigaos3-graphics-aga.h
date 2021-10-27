#ifndef PLATFORM_AMIGAOS3_GRAPHICS_AGA_H
#define PLATFORM_AMIGAOS3_GRAPHICS_AGA_H

#include <proto/graphics.h>
#include <proto/intuition.h>

namespace amigaos3 {

class NativeScreen {
public:
	NativeScreen() {
	}

private:
	/** Hardware window */
	struct Window *_hardwareWindow;

	/** Hardware screen */
	static const UBYTE AGA_VIDEO_DEPTH = 8;
	static const UBYTE NUM_SCREENBUFFERS = 2;

	struct BitMap *_linearBitmaps[NUM_SCREENBUFFERS];
	struct BitMap _screenBitmaps[NUM_SCREENBUFFERS];
	struct ScreenBuffer *_hardwareScreenBuffer[NUM_SCREENBUFFERS];
	struct RastPort _screenRastPorts[NUM_SCREENBUFFERS];
	UBYTE _currentScreenBuffer = 0;

	struct Screen *_hardwareScreen;
};

} // namespace amigaos3

#endif // AMIGAOS3GRAPHICSAGA_H
