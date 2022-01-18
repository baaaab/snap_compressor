#include "CSdlDisplay.h"

#include <stdio.h>
#include <X11/Xlib.h>
#include <mutex>
#include <math.h>
#include <unistd.h>
#include <algorithm>

#include "ASdlKeyPressHandler.h"

CSdlDisplay::CSdlDisplay(uint32_t width, uint32_t height) :
		_screen(NULL),
		_windowWidth(width),
		_windowHeight(height)
{
	XInitThreads();

	if (SDL_Init( SDL_INIT_VIDEO) != 0)
	{
		fprintf(stderr, "Could not initialise SDL: %s\n", SDL_GetError());
	}

	_screen = SDL_CreateWindow("wave_cmp",
	                          SDL_WINDOWPOS_UNDEFINED,
	                          SDL_WINDOWPOS_UNDEFINED,
	                          _windowWidth, _windowHeight,
	                          0);

	_renderer = SDL_CreateRenderer(_screen, -1, 0);

	_texture = SDL_CreateTexture(_renderer,
	                               SDL_PIXELFORMAT_ARGB8888,
	                               SDL_TEXTUREACCESS_STREAMING,
	                               _windowWidth, _windowHeight);

	_pixels = new uint8_t[_windowWidth * _windowHeight * 4];
}

CSdlDisplay::~CSdlDisplay()
{
	delete[] _pixels;
	SDL_DestroyTexture(_texture);
	SDL_DestroyRenderer(_renderer);
	SDL_DestroyWindow(_screen);
}

uint32_t CSdlDisplay::getWidth() const
{
	return _windowWidth;
}

uint32_t CSdlDisplay::getHeight() const
{
	return _windowHeight;
}

void CSdlDisplay::setPixel(int x, int y, uint32_t pixel)
{
	uint8_t *target_pixel = (uint8_t *) _pixels + y * _windowWidth * 4 + x * 4;
	*(uint32_t *) target_pixel = pixel;
}

uint32_t CSdlDisplay::setColour(uint8_t r, uint8_t g, uint8_t b)
{
	uint32_t pixel = b + (g << 8) + (r << 16);
	return pixel;
}

void CSdlDisplay::drawStraightLine(int x1, int y1, int x2, int y2, uint32_t pixel)
{
	if (y1 > y2)
	{
		int temp = y2;
		y2 = y1;
		y1 = temp;
	}
	if (x1 > x2)
	{
		int temp = x2;
		x2 = x1;
		x1 = temp;
	}
	if (x1 == x2)
	{
		for (int y = y1; y < y2 + 1; y++)
		{
			setPixel(x1, y, pixel);
		}
	}
	else
	{
		for (int x = x1; x < x2; x++)
		{
			setPixel(x, y1, pixel);
		}
	}
}

void CSdlDisplay::drawLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t pixel)
{
	int32_t dx = std::abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
	int32_t dy = std::abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
	int32_t err = (dx > dy ? dx : -dy) / 2, e2;

	for (;;)
	{
		setPixel(x1, y1, pixel);
		if (x1 == x2 && y1 == y2)
			break;
		e2 = err;
		if (e2 > -dx)
		{
			err -= dy;
			x1 += sx;
		}
		if (e2 < dy)
		{
			err += dx;
			y1 += sy;
		}
	}
}

void CSdlDisplay::swapBuffers()
{
	SDL_UpdateTexture(_texture, NULL, _pixels, _windowWidth * 4);

	SDL_RenderClear(_renderer);
	SDL_RenderCopy(_renderer, _texture, NULL, NULL);
	SDL_RenderPresent(_renderer);
}

uint8_t* CSdlDisplay::getPixels() const
{
	return _pixels;
}

void CSdlDisplay::addKeyHandler(ASdlKeyPressHandler* handler)
{
	std::lock_guard<std::mutex> am(_keyHandlersMutex);
	_keyHandlers.push_back(handler);
}

void CSdlDisplay::removeKeyHandler(ASdlKeyPressHandler* handler)
{
	std::lock_guard<std::mutex> am(_keyHandlersMutex);
	_keyHandlers.erase(std::remove(_keyHandlers.begin(), _keyHandlers.end(), handler), _keyHandlers.end());
}

void CSdlDisplay::handleEvents()
{
	while (SDL_PollEvent(&_event))
	{
		switch (_event.type)
		{
			case SDL_MOUSEMOTION:
			{
				//printf("Mouse moved by %d,%d to (%d,%d)\n", _event.motion.xrel, _event.motion.yrel,	_event.motion.x, _event.motion.y);
				break;
			}
			case SDL_MOUSEBUTTONDOWN:
			{
				//printf("Mouse button %d pressed at (%d,%d)\n", _event.button.button, _event.button.x, _event.button.y);
				break;
			}
			case SDL_KEYDOWN:
			{
				std::lock_guard<std::mutex> am(_keyHandlersMutex);
				for(ASdlKeyPressHandler* handler : _keyHandlers)
				{
					handler->handleKeyPress(_event.key.keysym.sym);
				}
				break;
			}
			case SDL_QUIT:
			{
				exit(0);
			}
		}
	}
}

