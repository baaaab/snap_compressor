#include "CXZCompress.h"

#include <cstdlib>
#include <cstring>

CXZCompress::CXZCompress()
{
	_strm = LZMA_STREAM_INIT;

	/* not present on my system :(
	 lzma_mt multiThreadingOptions;
	 multiThreadingOptions.flags = 0;
	 multiThreadingOptions.threads = 4;
	 multiThreadingOptions.block_size = 0; // let library set this
	 multiThreadingOptions.timeout = 1000; // ms
	 multiThreadingOptions.preset = 5;
	 multiThreadingOptions.check = LZMA_CHECK_CRC64;
	 lzma_ret ret = lzma_stream_encoder_mt(&_strm, &multiThreadingOptions)
	 */

	// 3: Encoding: 282.4 / 283.6 MB processed, compressed size: 157.5 MB, ratio: 55.79% (75.00% trimming, 74.39% xz), rate = 1.83 MB/s, eta: 0.670191s
	// 5: Encoding: 282.3 / 283.6 MB processed, compressed size: 147.4 MB, ratio: 52.22% (75.00% trimming, 69.62% xz), rate = 1.46 MB/s, eta: 0.889745s
	// 9: Encoding: 283.4 / 283.6 MB processed, compressed size: 148.1 MB, ratio: 52.27% (75.00% trimming, 69.70% xz), rate = 1.07 MB/s, eta: 0.181572s
	lzma_ret ret = lzma_easy_encoder(&_strm, 9, LZMA_CHECK_CRC64);
	switch (ret)
	{
		case LZMA_OK:
			break;
		case LZMA_MEM_ERROR:
			fprintf(stderr, "LZMA memory error\n");
			throw 1;
		case LZMA_OPTIONS_ERROR:
			fprintf(stderr, "Invalid LZMA options\n");
			throw 1;
		case LZMA_UNSUPPORTED_CHECK:
			fprintf(stderr, "LZMA unsupported\n");
			throw 1;
		case LZMA_PROG_ERROR:
			fprintf(stderr, "LZMA programming error\n");
			throw 1;
	}

	_bufferSize = 512 * 1024;
	_inputBuffer = (uint8_t*) malloc(_bufferSize);
	_outputBuffer = (uint8_t*) malloc(_bufferSize);
	_inputBufferUsed = 0;
	_isFinishing = false;

	_strm.next_out = _outputBuffer;
	_strm.avail_out = _bufferSize;
}

CXZCompress::~CXZCompress()
{
	lzma_end(&_strm);
	free(_inputBuffer);
	free(_outputBuffer);
}

void CXZCompress::addBytes(const uint8_t* data, uint32_t numBytes)
{
	if (_inputBufferUsed + numBytes < _bufferSize)
	{
		// add new data to buffer, to avoid so many calls into the lzma library
		memcpy(_inputBuffer + _inputBufferUsed, data, numBytes);
		_inputBufferUsed += numBytes;
	}
	else
	{
		_strm.next_in = _inputBuffer;
		_strm.avail_in = _inputBufferUsed;

		lzma_ret ret = lzma_code(&_strm, LZMA_RUN);

		switch (ret)
		{
			case LZMA_OK:
				//fprintf(stderr, "LZMA OK\n");
				break;
			case LZMA_STREAM_END:
				fprintf(stderr, "LZMA stream end\n");
				break;
			case LZMA_BUF_ERROR:
				fprintf(stderr, "LZMA buf error\n");
				break;
			case LZMA_MEM_ERROR:
				fprintf(stderr, "LZMA memory error\n");
				throw 1;
			case LZMA_OPTIONS_ERROR:
				fprintf(stderr, "Invalid LZMA options\n");
				throw 1;
			case LZMA_UNSUPPORTED_CHECK:
				fprintf(stderr, "LZMA unsupported\n");
				throw 1;
			case LZMA_PROG_ERROR:
				fprintf(stderr, "LZMA programming error\n");
				throw 1;
		}

		// move any remaining data back to the beginning of the buffer
		memmove(_inputBuffer, _strm.next_in, _strm.avail_in);
		_inputBufferUsed = _strm.avail_in;

		addBytes(data, numBytes);
	}
}

void CXZCompress::writeAndEmptyBuffer(FILE* fh)
{
	uint32_t bytesUsed = _bufferSize - _strm.avail_out;

	fwrite(_outputBuffer, 1, bytesUsed, fh);

	_strm.next_out = _outputBuffer;
	_strm.avail_out = _bufferSize;
}

bool CXZCompress::finish()
{
	if (!_isFinishing)
	{
		_strm.next_in = _inputBuffer;
		_strm.avail_in = _inputBufferUsed;
		_isFinishing = true;
	}

	lzma_ret ret = lzma_code(&_strm, LZMA_FINISH);

	return ret == LZMA_STREAM_END;
}

float CXZCompress::getRatio() const
{
	float ratio = _strm.total_out / (float) _strm.total_in;
	return ratio;
}

