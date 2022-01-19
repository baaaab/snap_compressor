#ifndef SRC_MATHS_CDISCRETECOSINETRANSFORM_H_
#define SRC_MATHS_CDISCRETECOSINETRANSFORM_H_

#include <complex>
#include <vector>

class CDiscreteCosineTransform
{
public:
	CDiscreteCosineTransform(uint32_t blockSize);
	~CDiscreteCosineTransform();

	static void DCT(const std::vector<std::complex<float>>& data, std::vector<std::complex<float>>& destination);
	static void IDCT(const std::vector<std::complex<float>>& data, std::vector<std::complex<float>>& destination);

	void optDCT(const std::vector<std::complex<float>>& data, std::vector<std::complex<float>>& destination);
	void optIDCT(const std::vector<std::complex<float>>& data, std::vector<std::complex<float>>& destination);

private:
	uint32_t _blockSize;
	float** _cosLookup;
};

#endif /* SRC_MATHS_CDISCRETECOSINETRANSFORM_H_ */
