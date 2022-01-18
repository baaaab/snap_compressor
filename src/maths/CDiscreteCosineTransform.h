#ifndef SRC_MATHS_CDISCRETECOSINETRANSFORM_H_
#define SRC_MATHS_CDISCRETECOSINETRANSFORM_H_

#include <complex>
#include <vector>

class CDiscreteCosineTransform
{
public:
	CDiscreteCosineTransform();
	~CDiscreteCosineTransform();

	static void DCT(const std::vector<std::complex<float>>& data, std::vector<std::complex<float>>& destination);
	static void IDCT(const std::vector<std::complex<float>>& data, std::vector<std::complex<float>>& destination);
};

#endif /* SRC_MATHS_CDISCRETECOSINETRANSFORM_H_ */
