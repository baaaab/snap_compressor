#include "CDiscreteCosineTransform.h"

CDiscreteCosineTransform::CDiscreteCosineTransform(uint32_t blockSize) :
	_blockSize(blockSize),
	_cosLookup(nullptr)
{
	const float scaledPi = M_PI / (float)blockSize;

	// the cos lookup is the slowest part of this O(n^2) algorithm, so create a lookup table instead.
	_cosLookup = new float*[_blockSize];
	for(uint32_t a=0;a<blockSize;a++)
	{
		_cosLookup[a] = new float[_blockSize];
		for(uint32_t b=0;b<_blockSize;b++)
		{
			_cosLookup[a][b] = cosf(scaledPi * (b + 0.5f) * a);
		}
	}
}

CDiscreteCosineTransform::~CDiscreteCosineTransform()
{
	for(uint32_t i=0;i<_blockSize;i++)
	{
		delete[] _cosLookup[i];
	}
	delete[] _cosLookup;
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
}

void CDiscreteCosineTransform::optDCT(const std::vector<std::complex<float>>& data, std::vector<std::complex<float>>& destination)
{
	destination.resize(data.size());

	const float scalingFactor = sqrtf(2.0f / data.size());

	for (uint32_t a = 0; a < _blockSize; a++)
	{
		std::complex<float> sum = 0;
		for (uint32_t b = 0; b < _blockSize; b++)
		{
			sum += data[b] * _cosLookup[a][b];
		}
		destination[a] = sum * scalingFactor;
	}
	destination[0] *= 1.0f / sqrtf(2);
}

void CDiscreteCosineTransform::optIDCT(const std::vector<std::complex<float>>& data, std::vector<std::complex<float>>& destination)
{
	destination.resize(data.size());

	const float scalingFactor = sqrtf(2.0f / data.size());

	for (uint32_t a = 0; a < _blockSize; a++)
	{
		std::complex<float> sum = data[0] * 0.5f;
		for (uint32_t b = 1; b < _blockSize; b++)
		{
			sum += data[b] * _cosLookup[b][a];
		}
		destination[a] = sum * scalingFactor;
	}
}

