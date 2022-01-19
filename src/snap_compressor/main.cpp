#include <cinttypes>
#include <complex>
#include <cstdio>
#include <cstdlib>
#include <cwchar>
#include <vector>
#include <cstring>
#include <ctime>
#include <sys/stat.h>

#include "CDiscreteCosineTransform.h"
#include "CXZCompress.h"
#include "CXZDecompress.h"

void encode(const char* inputFileName, uint32_t blockSize, float quantisationFactor, uint32_t binsToKeep);
void decode(const char* inputFileName, const char* outputFileName);

namespace
{
const uint8_t magic[4] = {0xB0, 0xBD, 0xC7, 0x01};
}

void usage(const char* argv0)
{
	fprintf(stderr, "Usage: %s encode snapshot.8t block_size quantisation_percent cut_off_freq_percent\n", argv0);
	fprintf(stderr, "Usage: %s decode encoded.roundedQuantisedDCT decoded.8t\n", argv0);
	fprintf(stderr, "\tquantisation_percent is a scaling factor applied to all DCT values, to help with future entropy encoding\n");
	fprintf(stderr, "\tblock_size is the DCT size, larger values may give better compression, but operation is O(n^2)\n");
	fprintf(stderr, "\tcut_off_freq_percent can be used to filter high frequency components, specify the bandwidth percent to preserve\n");
	exit(1);
}

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		usage(argv[0]);
	}

	if (strcmp(argv[1], "encode") == 0)
	{
		if(argc != 6)
		{
			usage(argv[0]);
		}
		const char* inputFileName = argv[2];
		uint32_t blockSize = strtoul(argv[3], NULL, 10);
		float quantisationFactor = strtof(argv[4], NULL) / 100.0f;
		float cutOffFreq = strtof(argv[5], NULL) / 100.0f;
		uint32_t binsToKeep = ceilf(blockSize * cutOffFreq);

		encode(inputFileName, blockSize, quantisationFactor, binsToKeep);
	}
	else if (strcmp(argv[1], "decode") == 0)
	{
		if(argc != 4)
		{
			usage(argv[0]);
		}
		const char* inputFileName = argv[2];
		const char* ouputFileName = argv[3];

		decode(inputFileName, ouputFileName);
	}
	else
	{
		usage(argv[0]);
	}
}

void encode(const char* inputFileName, uint32_t blockSize, float quantisationFactor, uint32_t binsToKeep)
{

	// todo: figure out non const values?
	std::vector<float> quantisationVector(blockSize, quantisationFactor);

	FILE* fh = fopen(inputFileName, "r");
	if (!fh)
	{
		fprintf(stderr, "Cannot read: '%s'\n", inputFileName);
		exit(1);
	}

	FILE* roundedQuantisedDct = NULL;
	uint64_t fileSizeBytes = 0;

	{
		char tmpFileName[1024];

		snprintf(tmpFileName, sizeof(tmpFileName), "%s.roundedQuantisedDCT", inputFileName);
		roundedQuantisedDct = fopen(tmpFileName, "w");

		struct stat st;
		stat(inputFileName, &st);
		fileSizeBytes = st.st_size;
	}



	CXZCompress compressor;
	CDiscreteCosineTransform dct(blockSize);

	//write headers (magic, blockSize, quantisationFactor, binsToKeep) all 4 bytes, LE
	{
		// these aren't compressed to make it easier to see what is going on (but means we can't use xz to decompress)
		fwrite(magic, 1, 4, roundedQuantisedDct);

		fwrite(&blockSize, 4, 1, roundedQuantisedDct);

		fwrite(&quantisationFactor, 4, 1, roundedQuantisedDct);

		fwrite(&binsToKeep, 4, 1, roundedQuantisedDct);
	}

	std::vector<std::complex<int8_t>> bytes(blockSize);
	std::vector<std::complex<float>> floats(blockSize);
	std::vector<std::complex<float>> transformed;
	std::vector<std::complex<int8_t>> transformedRounded(blockSize);
	std::vector<std::complex<int8_t>> transformedRoundedQuantised(blockSize);

	time_t start = time(NULL);
	time_t lastPrint = start;
	uint64_t bytesProcessed = 0;

	while (!feof(fh))
	{
		if (fread(bytes.data(), 2, blockSize, fh) == blockSize)
		{
			for (uint32_t i = 0; i < blockSize; i++)
			{
				floats[i] = bytes[i];
			}

			bytesProcessed += blockSize * 2;

			dct.optDCT(floats, transformed);

			for (uint32_t i = 0; i < blockSize; i++)
			{
				if(std::abs(transformed[i].real()) * quantisationVector[i] > 127)
				{
					fprintf(stderr, "Overflow detected, set quantisation to: %f %%\n", quantisationFactor * 12700.0f / (quantisationVector[i] * std::abs(transformed[i].real())));
				}
				if(std::abs(transformed[i].imag()) * quantisationVector[i] > 127)
				{
					fprintf(stderr, "Overflow detected, set quantisation to: %f %%\n", quantisationFactor * 12700.0f / (quantisationVector[i] * std::abs(transformed[i].imag())));
				}

				transformedRounded[i].real(roundf(transformed[i].real()));
				transformedRounded[i].imag(roundf(transformed[i].imag()));

				transformedRoundedQuantised[i].real(roundf(transformed[i].real() * quantisationVector[i]));
				transformedRoundedQuantised[i].imag(roundf(transformed[i].imag() * quantisationVector[i]));
			}

			//fwrite(transformedRoundedQuantised.data(), 2, binsToKeep, roundedQuantisedDct);
			compressor.addBytes(reinterpret_cast<uint8_t*>(transformedRoundedQuantised.data()), 2 * binsToKeep);
			compressor.writeAndEmptyBuffer(roundedQuantisedDct);

			if(time(NULL) != lastPrint)
			{
				lastPrint = time(NULL);
				float megaBytesProcessed = bytesProcessed / 1000000.0f;
				float megaBytesOutput = ftell(roundedQuantisedDct) / 1000000.0f;
				float ratioFromCuttingHighFreqs = binsToKeep / (float)blockSize;
				float xzRatio = compressor.getRatio();
				float overallRatio = ratioFromCuttingHighFreqs * xzRatio;
				float rate = megaBytesProcessed / (float)(lastPrint - start);
				float fileSizeMegaBytes = fileSizeBytes / 1000000.0f;
				float eta = (fileSizeMegaBytes - megaBytesProcessed)/ rate;
				printf("Encoding: %3.1f / %3.1f MB processed, compressed size: %3.1f MB, ratio: %2.2f%% (%2.2f%% trimming, %2.2f%% xz), rate = %2.2f MB/s, eta: %3.0f s\n", megaBytesProcessed, fileSizeMegaBytes, megaBytesOutput, overallRatio * 100.0f, ratioFromCuttingHighFreqs * 100.0f, xzRatio * 100.0f, rate, eta);
			}

			/*for (uint32_t i = 0; i < blockSize; i++)
			 {
			 printf("%03d: (%3d, %3d) -> (%3d, %3d) -> (%3d, %3d)\n",
			 i,
			 bytes[i].real(), bytes[i].imag(),
			 transformedRounded[i].real(), transformedRounded[i].imag(),
			 transformedRoundedQuantised[i].real(), transformedRoundedQuantised[i].imag()
			 );
			 }
			 printf("###################################\n");*/
		}
	}

	bool done = false;
	while(!done)
	{
		done = compressor.finish();
		compressor.writeAndEmptyBuffer(roundedQuantisedDct);
	}

	fclose(roundedQuantisedDct);
}

void decode(const char* inputFileName, const char* outputFileName)
{
	FILE* inputFh = fopen(inputFileName, "r");
	if (!inputFh)
	{
		fprintf(stderr, "Cannot read: '%s'\n", inputFileName);
		exit(1);
	}

	FILE* decodedFh = fopen(outputFileName, "w");
	if (!decodedFh)
	{
		fprintf(stderr, "Cannot write: '%s'\n", outputFileName);
		exit(1);
	}

	uint64_t fileSizeBytes = 0;
	{
		struct stat st;
		stat(inputFileName, &st);
		fileSizeBytes = st.st_size;
	}

	char fileMagic[4];
	uint32_t blockSize;
	float quantisationFactor;
	uint32_t binsToKeep;

	//read headers (magic, blockSize, quantisationFactor, binsToKeep) all 4 bytes, LE
	{
		fread(fileMagic, 1, 4, inputFh);
		if(memcmp(fileMagic, magic, 4) != 0)
		{
			fprintf(stderr, "Invalid file header\n");
			exit(1);
		}

		fread(&blockSize, 4, 1, inputFh);
		if(blockSize == 0)
		{
			fprintf(stderr, "block size of zero is invalid\n");
			exit(1);
		}

		fread(&quantisationFactor, 4, 1, inputFh);
		if(quantisationFactor == 0.0f)
		{
			fprintf(stderr, "quantisationFactor of zero is invalid\n");
			exit(1);
		}
		if(quantisationFactor > 1.0f)
		{
			fprintf(stderr, "quantisationFactor above 1 is a bad idea\n");
		}

		fread(&binsToKeep, 4, 1, inputFh);
		if(binsToKeep == 0 || binsToKeep > blockSize)
		{
			fprintf(stderr, "Invalid binsToKeep = %u, blockSize = %u\n", binsToKeep, blockSize);
			exit(1);
		}

		fprintf(stderr, "Read header successfully. Blocksize = %u, quantisation = %f, binsToKeep = %u\n", blockSize, quantisationFactor, binsToKeep);
	}

	CXZDecompress decompressor(inputFh);
	CDiscreteCosineTransform dct(blockSize);

	std::vector<std::complex<float>> inverseTransformed;
	std::vector<std::complex<float>> rounded(blockSize, {0.0f, 0.0f});
	std::vector<std::complex<int8_t>> iBytes(blockSize, {0,0});

	std::vector<std::complex<int8_t>> bytes(blockSize, {0,0});
	std::vector<std::complex<float>> floats(blockSize, {0.0f, 0.0f});

	time_t start = time(NULL);
	time_t lastPrint = start;

	while (!feof(inputFh) && decompressor.consumeBytes(reinterpret_cast<char*>(bytes.data()), 2 * binsToKeep))
	{
		//if (fread(bytes.data(), 2, binsToKeep, inputFh) == binsToKeep)
		{
			for (uint32_t i = 0; i < blockSize; i++)
			{
				floats[i] = bytes[i];
				floats[i] /= quantisationFactor;
			}

			dct.optIDCT(floats, inverseTransformed);

			for (uint32_t i = 0; i < blockSize; i++)
			{
				rounded[i].real(roundf(inverseTransformed[i].real()));
				rounded[i].imag(roundf(inverseTransformed[i].imag()));
			}

			for (uint32_t i = 0; i < blockSize; i++)
			{
				iBytes[i] = rounded[i];
			}

			if(time(NULL) != lastPrint)
			{
				lastPrint = time(NULL);
				float megaBytesCompressed = decompressor.getInputByteCount() / 1000000.0f;
				float megaBytesDecompressed = decompressor.getOutputByteCount() / 1000000.0f;
				float ratioFromCuttingHighFreqs = binsToKeep / (float)blockSize;
				float xzRatio = megaBytesCompressed / megaBytesDecompressed;
				float overallRatio = ratioFromCuttingHighFreqs * xzRatio;
				float rate = megaBytesCompressed / (float)(lastPrint - start);
				float fileSizeMegaBytes = fileSizeBytes / 1000000.0f;
				float eta = (fileSizeMegaBytes - megaBytesCompressed)/ rate;
				printf("Decoding: %3.1f / %3.1f MB processed, decompressed size: %3.1f MB, ratio: %2.2f%% (%2.2f%% trimming, %2.2f%% xz), (input)rate = %2.2f MB/s, eta: %3.0f s\n", megaBytesCompressed, fileSizeMegaBytes, megaBytesDecompressed, overallRatio * 100.0f, ratioFromCuttingHighFreqs * 100.0f, xzRatio * 100.0f, rate, eta);
			}

			fwrite(iBytes.data(), 2, blockSize, decodedFh);
		}
	}
}
