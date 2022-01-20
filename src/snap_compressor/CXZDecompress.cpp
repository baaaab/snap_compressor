#include "CXZDecompress.h"

#include <cstdlib>
#include <cstring>

CXZDecompress::CXZDecompress(FILE* fh) :
		_dataSource(fh)
{
	_strm = LZMA_STREAM_INIT;

	lzma_ret ret = lzma_stream_decoder(&_strm, 1e9, LZMA_TELL_UNSUPPORTED_CHECK);
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

	_bufferSize = 1024 * 1024;
	_inputBuffer = (uint8_t*) malloc(_bufferSize);
	_outputBuffer = (uint8_t*) malloc(_bufferSize);
	_outputBufferReadPointer = _outputBuffer;

	_strm.next_out = _outputBuffer;
	_strm.avail_out = _bufferSize;

	_strm.next_in = _inputBuffer;
	_strm.avail_in = 0;
}

CXZDecompress::~CXZDecompress()
{
	lzma_end(&_strm);
	free(_inputBuffer);
	free(_outputBuffer);
}

uint32_t CXZDecompress::getNumDecompressedBytesAvailable() const
{
	return _strm.next_out - _outputBufferReadPointer;
}

size_t CXZDecompress::getInputByteCount() const
{
	return _strm.total_in;
}

size_t CXZDecompress::getOutputByteCount() const
{
	return _strm.total_out;
}

bool CXZDecompress::consumeBytes(char* data, uint32_t bytesToRead)
{
	while (getNumDecompressedBytesAvailable() < bytesToRead)
	{
		resetOutputBuffer();

		if (_strm.avail_in == 0)
		{
			if (!readDataFromFile())
			{
				// no more data available
				return false;
			}
		}

		lzma_ret ret = lzma_code(&_strm, LZMA_RUN);

		switch (ret)
		{
			case LZMA_OK:
//				fprintf(stderr, "LZMA OK\n");
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
			case LZMA_DATA_ERROR:
				fprintf(stderr, "LZMA data is corrupted\n");
				throw 1;
			case LZMA_PROG_ERROR:
				fprintf(stderr, "LZMA programming error\n");
				throw 1;
		}
	}

	memcpy(data, _outputBufferReadPointer, bytesToRead);
	_outputBufferReadPointer += bytesToRead;

	return true;

}

void CXZDecompress::resetOutputBuffer()
{
	uint32_t bytesAvailable = getNumDecompressedBytesAvailable();
	memmove(_outputBuffer, _outputBufferReadPointer, bytesAvailable);
	_strm.next_out = _outputBuffer + bytesAvailable;
	_strm.avail_out = _bufferSize;
	_outputBufferReadPointer = _outputBuffer;
}

bool CXZDecompress::readDataFromFile()
{
	// move existing data back to the front of the buffer
	memmove(_inputBuffer, _strm.next_in, _strm.avail_in);
	_strm.next_in = _inputBuffer;

	int bytesRead = fread(_inputBuffer + _strm.avail_in, 1, _bufferSize - _strm.avail_in, _dataSource);
	if (bytesRead > 0)
	{
		_strm.avail_in += bytesRead;
	}

	if (bytesRead <= 0)
	{
		return false;
	}
	return true;
}

