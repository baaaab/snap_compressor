#ifndef SRC_SNAP_COMPRESSOR_CXZCOMPRESS_H_
#define SRC_SNAP_COMPRESSOR_CXZCOMPRESS_H_

#include <lzma.h>
#include <stdio.h>

class CXZCompress
{
public:
	CXZCompress();
	virtual ~CXZCompress();

	void addBytes(const uint8_t* data, uint32_t numBytes);
	void writeAndEmptyBuffer(FILE* fh);

	// call repeatedly until it returns true, write after each call
	bool finish();

	float getRatio() const;

private:
	uint8_t* _outputBuffer;
	uint32_t _bufferSize;
	lzma_stream _strm;
};

#endif /* SRC_SNAP_COMPRESSOR_CXZCOMPRESS_H_ */
