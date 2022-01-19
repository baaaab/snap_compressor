#ifndef SRC_SNAP_COMPRESSOR_CXZDECOMPRESS_H_
#define SRC_SNAP_COMPRESSOR_CXZDECOMPRESS_H_

#include <lzma.h>
#include <stdio.h>

class CXZDecompress
{
public:
	CXZDecompress(FILE* fh);
	virtual ~CXZDecompress();

	uint32_t getNumDecompressedBytesAvailable() const;
	bool consumeBytes(char* data, uint32_t bytesToRead);

	size_t getInputByteCount() const;
	size_t getOutputByteCount() const;

private:
	void resetOutputBuffer();
	bool readDataFromFile();

	FILE* _dataSource;

	uint8_t* _inputBuffer;
	uint8_t* _outputBuffer;
	uint8_t* _outputBufferReadPointer;
	uint32_t _bufferSize;
	lzma_stream _strm;
};

#endif /* SRC_SNAP_COMPRESSOR_CXZCOMPRESS_H_ */
