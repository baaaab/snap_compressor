#include "CDiscreteCosineTransform.h"

CDiscreteCosineTransform::CDiscreteCosineTransform()
{

}

CDiscreteCosineTransform::~CDiscreteCosineTransform()
{

}

void CDiscreteCosineTransform::DCT(const std::vector<std::complex<float>>& data, std::vector<std::complex<float>>& destination)
{
	destination.resize(data.size());

	const float scaledPi = M_PI / (float)data.size();

	for (uint32_t a = 0; a < data.size(); a++)
	{
		std::complex<float> sum = 0;
		for (uint32_t b = 0; b < data.size(); b++)
		{
			sum += data[b] * cosf(scaledPi * (b + 0.5f) * a);
		}
		destination[a] = sum * sqrtf(2.0f / data.size());
	}
	destination[0] *= 1.0f / sqrtf(2);
}

void CDiscreteCosineTransform::IDCT(const std::vector<std::complex<float>>& data, std::vector<std::complex<float>>& destination)
{
	destination.resize(data.size());

	const float scaledPi = M_PI / (float)data.size();

	for (uint32_t a = 0; a < data.size(); a++)
	{
		std::complex<float> sum = data[0] * 0.5f;
		for (uint32_t b = 1; b < data.size(); b++)
		{
			sum += data[b] * cosf(scaledPi * (a + 0.5f) * b);
		}
		destination[a] = sum * sqrtf(2.0f / data.size());
	}
	//destination[0] *= 1.0f / sqrtf(2);
}

