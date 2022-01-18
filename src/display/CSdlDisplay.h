#ifndef CSDLDISPLAY_H_
#define CSDLDISPLAY_H_

#include <SDL2/SDL.h>
#include <thread>
#include <mutex>
#include <vector>

class ASdlKeyPressHandler;

class CSdlDisplay
{
public:
	CSdlDisplay(uint32_t width, uint32_t height);
	virtual ~CSdlDisplay();

	uint32_t    getWidth             () const;
	uint32_t    getHeight            () const;

	void        setPixel             (int32_t x, int32_t y, uint32_t pixel);
	void        drawStraightLine     (int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t pixel);
	void        drawLine             (int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t pixel);
	uint32_t    setColour            (uint8_t r, uint8_t g, uint8_t b);

	void        swapBuffers          ();

	uint8_t*    getPixels            () const;

	void        addKeyHandler        (ASdlKeyPressHandler* handler);
	void        removeKeyHandler     (ASdlKeyPressHandler* handler);

	void        handleEvents         ();

private:

	SDL_Window* _screen;
	SDL_Renderer* _renderer;
	SDL_Texture* _texture;
	uint8_t*     _pixels;

	SDL_Event _event;
	uint32_t _windowWidth;
	uint32_t _windowHeight;

	std::vector<ASdlKeyPressHandler*> _keyHandlers;
	std::mutex                        _keyHandlersMutex;



};

#endif /* CDISPLAY_H_ */
