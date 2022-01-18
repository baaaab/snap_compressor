#ifndef SRC_FFT_ASDLKEYPRESSHANDLER_H_
#define SRC_FFT_ASDLKEYPRESSHANDLER_H_

#include <SDL2/SDL.h>

class ASdlKeyPressHandler
{
public:
	ASdlKeyPressHandler();
	virtual ~ASdlKeyPressHandler();

	virtual void        handleKeyPress(SDL_Keycode) = 0;

};

#endif /* SRC_FFT_ASDLKEYPRESSHANDLER_H_ */
