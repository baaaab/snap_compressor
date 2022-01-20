#include <stdio.h>
#include <cstdint>
#include <cstdlib>
#include <cwchar>
#include <vector>
#include <complex>
#include <algorithm>
#include <unistd.h>

#include "../src/display/ASdlKeyPressHandler.h"
#include "../src/display/CSdlDisplay.h"
#include "../src/fft/kiss_fft.h"

class CKeyPressHandler: public ASdlKeyPressHandler
{
public:
	CKeyPressHandler()
	{
	}

	void handleKeyPress(SDL_Keycode key)
	{
		switch(key)
		{
			case SDLK_RIGHT:
			{
				doRedraw = true;
				segment++;
				break;
			}
			case SDLK_LEFT:
			{
				doRedraw = true;
				if(segment != 0)
				{
					segment--;
				}
				break;
			}
			case SDLK_s:
			{
				doRedraw = true;
				drawOrder[0] = 1 - drawOrder[0];
				drawOrder[1] = 1 - drawOrder[1];
				break;
			}
		}
	}

	bool doRedraw = false;
	uint32_t segment = 0;
	int drawOrder[2] = {0, 1};
};

void drawConstellation(CSdlDisplay* display, uint32_t xOffset, uint32_t yOffset, const std::vector<std::complex<int8_t>>& samples, uint32_t numSamples, uint32_t colour);
void drawTimeDomain(CSdlDisplay* display, uint32_t xOffset, uint32_t yOffset, const std::vector<std::complex<int8_t>>& samples, uint32_t colour);
void drawFft(CSdlDisplay* display, uint32_t xOffset, uint32_t yOffset, const std::vector<std::complex<int8_t>>& samples, uint32_t fftSize, uint32_t colour);
void drawPhaseError(CSdlDisplay* display, uint32_t xOffset, uint32_t yOffset, const std::vector<std::complex<int8_t>>* samples, uint32_t colour);

int main(int argc, char** argv)
{
	if (argc != 3)
	{
		fprintf(stderr, "Usage: %s file1.8t file2.8t\n", argv[0]);
		exit(1);
	}

	FILE* fh[2];

	fh[0] = fopen(argv[1], "r");
	if (!fh[0])
	{
		fprintf(stderr, "Cannot read: '%s'\n", argv[1]);
		exit(1);
	}

	fh[1] = fopen(argv[2], "r");
	if (!fh[1])
	{
		fprintf(stderr, "Cannot read: '%s'\n", argv[1]);
		exit(1);
	}

	CSdlDisplay display(1900, 768);

	CKeyPressHandler keyPressHandler;
	display.addKeyHandler(&keyPressHandler);

	const uint32_t numSamples = display.getWidth() - 256;

	std::vector<std::complex<int8_t>> samples[2];
	samples[0].resize(numSamples*20);
	samples[1].resize(numSamples*20);

	std::vector<std::complex<int8_t>> errors(samples[0].size());

	while (1)
	{
		printf("Segment; %u, byteOffset: %lu\n", keyPressHandler.segment, keyPressHandler.segment * numSamples * 2ULL);
		for(uint32_t i=0;i<2;i++)
		{
			fseek(fh[i], keyPressHandler.segment * numSamples * 2ULL, SEEK_SET);
			fread(samples[i].data(), 2, numSamples*20, fh[i]);
		}

		for(uint32_t i=0;i<errors.size();i++)
		{
			errors[i] = samples[1][i] - samples[0][i];
		}

		uint8_t* pixels = display.getPixels();
		memset(pixels, 0, display.getWidth() * display.getHeight() * 4);

		uint32_t colour[3];
		colour[0] = display.setColour(0, 255, 0);
		colour[1] = display.setColour(255, 0, 0);
		colour[2] = display.setColour(0, 128, 255);

		drawConstellation(&display, 0, 0, samples[0], 1024, colour[0]);
		drawConstellation(&display, 0, 256, samples[1], 1024, colour[1]);
		drawConstellation(&display, 0, 512, errors, 1024, colour[2]);

		for (int i = 0; i < 2; i++)
		{
			int index = keyPressHandler.drawOrder[i];

			drawTimeDomain(&display, 256, 0, samples[index], colour[index]);
			drawFft(&display, 256, 256, samples[index], display.getWidth() - 256, colour[index]);
		}
		drawFft(&display, 256, 256, errors, display.getWidth() - 256, colour[2]);
		drawPhaseError(&display, 256, 512, samples, colour[2]);

		display.swapBuffers();

		while (!keyPressHandler.doRedraw)
		{
			usleep(40000);
			display.handleEvents();
		}
		keyPressHandler.doRedraw = false;
	}

	fclose (fh[0]);
	fclose (fh[1]);


}

void drawConstellation(CSdlDisplay* display, uint32_t xOffset, uint32_t yOffset, const std::vector<std::complex<int8_t>>& samples, uint32_t numSamples, uint32_t colour)
{
	for(uint32_t i=0;i<numSamples; i++)
	{
		const std::complex<int8_t>& point = samples[i];
		int x = point.real() + 128 + xOffset;
		int y = point.imag() + 128 + yOffset;
		display->setPixel(x, y, colour);
	}
}

void drawTimeDomain(CSdlDisplay* display, uint32_t xOffset, uint32_t yOffset, const std::vector<std::complex<int8_t>>& samples, uint32_t colour)
{
	uint32_t samplesToDraw = std::min((uint32_t)samples.size(), display->getWidth() - xOffset);
	uint32_t prevX = xOffset, prevY = yOffset + 128;
	for(uint32_t i=0;i<samplesToDraw;i++)
	{
		int real = 255 - (samples[i].real() + 128);
		uint32_t y = yOffset + real;
		uint32_t x = xOffset + i;

		display->drawLine(x, y, prevX, prevY, colour);

		prevX = x;
		prevY = y;
	}
}

void drawFft(CSdlDisplay* display, uint32_t xOffset, uint32_t yOffset, const std::vector<std::complex<int8_t>>& samples, uint32_t fftSize, uint32_t colour)
{
	std::vector<std::complex<float>> f(samples.size());
	for(uint32_t i=0;i<samples.size();i++)
	{
		f[i].real(samples[i].real());
		f[i].imag(samples[i].imag());
	}

	std::vector<std::complex<float>> fft(samples.size());

	kiss_fft_cfg cfg = kiss_fft_alloc( fftSize , false ,0,0 );

	uint32_t numAverages = samples.size() / fftSize;

	for(uint32_t n=0;n<numAverages;n++)
	{
		kiss_fft( cfg , reinterpret_cast<kiss_fft_cpx*>(f.data() + n * fftSize) , reinterpret_cast<kiss_fft_cpx*>(fft.data() + n * fftSize) );
	}

	free(cfg);

	std::vector<float> magnitudes(fftSize);

	for(uint32_t n=0;n<numAverages;n++)
	{
		for(uint32_t i=0;i<fftSize;i++)
		{
			magnitudes[(i + fftSize / 2) % fftSize] += log10f(std::abs(fft[i + n * fftSize]) + 1);
		}
	}

	for(uint32_t i=0;i<fftSize;i++)
	{
		magnitudes[i] /= numAverages;
	}

	float max = 4.5f;//*std::max_element(magnitudes.begin(), magnitudes.end());
	//printf("max: %f, numAverages = %u\n", max, numAverages);

	uint32_t prevX = xOffset, prevY = yOffset + 128;
	for(uint32_t i=0;i<fftSize;i++)
	{
		float magnitude =  magnitudes[i];
		if(magnitude < 0)
		{
			magnitude = 0;
		}
		magnitude = 255 - (255 * magnitude / max);
		uint32_t y = yOffset + roundf(magnitude);
		uint32_t x = xOffset + i;

		display->drawLine(x, y, prevX, prevY, colour);

		prevX = x;
		prevY = y;
	}
}

void drawPhaseError(CSdlDisplay* display, uint32_t xOffset, uint32_t yOffset, const std::vector<std::complex<int8_t>>* samples, uint32_t colour)
{
	uint32_t samplesToDraw = std::min((uint32_t)samples[0].size(), display->getWidth() - xOffset);
	uint32_t prevX = xOffset, prevY = yOffset + 128;
	for(uint32_t i=0;i<samplesToDraw;i++)
	{
		float phase1 = atan2f(samples[0][i].real(), samples[0][i].imag()) / (2 * M_PI);
		float phase2 = atan2f(samples[1][i].real(), samples[1][i].imag()) / (2 * M_PI);
		float mag1 = std::abs(samples[0][i]);
		float mag2 = std::abs(samples[0][i]);
		float phaseError = 127 + (phase2 - phase1) * (mag1 + mag2);
		while (phaseError < 0)
		{
			phaseError += 256;
		}
		while (phaseError > 255)
		{
			phaseError -= 256;
		}
		uint32_t y = yOffset + roundf(phaseError);
		uint32_t x = xOffset + i;


		/*printf("%4d, (%3d, %3d)(%3d, %3d) => %3.0f*%3.0f - %3.0f*%3.0f = %3.0f\n",
				i,
				samples[0][i].real(), samples[0][i].imag(),
				samples[1][i].real(), samples[1][i].imag(),
				phase1, mag1,
				phase2, mag2,
				phaseError
				);*/

		display->drawLine(x, y, prevX, prevY, colour);

		prevX = x;
		prevY = y;
	}
}
