#include <complex>
#include <vector>
#include <stdio.h>


int main(int argc, char** argv)
{
	if(argc != 3)
	{
		fprintf(stderr, "Usage: %s filename.sdriq [output.8t]\n", argv[0]);
		exit(1);
	}

	FILE* fh = fopen(argv[1], "r");
	if(!fh)
	{
		fprintf(stderr, "Cannot read: '%s'\n", argv[1]);
		exit(1);
	}

	FILE* output = fopen(argv[2], "w");
	if(!output)
	{
		fprintf(stderr, "Cannot write to: '%s'\n", argv[2]);
		exit(1);
	}

	{
		uint32_t sampleRate;
		uint64_t centreFreqHz;
		uint64_t timestamp;
		uint32_t sampleSize;
		uint32_t filler;
		uint32_t crc;

		fread(&sampleRate, sizeof(sampleRate), 1, fh);
		fread(&centreFreqHz, sizeof(centreFreqHz), 1, fh);
		fread(&timestamp, sizeof(timestamp), 1, fh);
		fread(&sampleSize, sizeof(sampleSize), 1, fh);
		fread(&filler, sizeof(filler), 1, fh);
		fread(&crc, sizeof(crc), 1, fh);

		fprintf(stderr, "sampleRate = %u\n", sampleRate);
		fprintf(stderr, "centreFreqHz = %lu\n", centreFreqHz);
		fprintf(stderr, "timestamp = %lu\n", timestamp);
		fprintf(stderr, "sampleSize = %u\n", sampleSize);
		fprintf(stderr, "filler = %u\n", filler);
		fprintf(stderr, "crc = %u\n", crc);
	}

	// pass 1 find max sample

	int32_t maxSample = 0;
	uint64_t numSamples = 0;

	while(!feof(fh))
	{
		int32_t sample[2];
		fread(&sample, 2, 4, fh);
		//printf("(%d, %d)\n", sample[0], sample[1]);
		for(uint32_t i=0;i<2;i++)
		{
			if(abs(sample[i]) > maxSample)
			{
				maxSample = abs(sample[i]);
			}
		}
		numSamples++;
	}

	printf("Max sample is: %d, numSamples = %lu\n", maxSample, numSamples);

	fclose(fh);
	fh = fopen(argv[1], "r");
	// skip header
	fseek(fh, 32, SEEK_SET);

	float scalingFactor = 127.0f / (float)maxSample;

	for(uint64_t i=0;i<numSamples;i++)
	{
		int32_t sample[2];
		fread(&sample, 2, 4, fh);
		int8_t scaled[2];
		scaled[0] = sample[0] * scalingFactor;
		scaled[1] = sample[1] * scalingFactor;

		fwrite(scaled, 1, 2, output);
	}

	fclose(output);

}
