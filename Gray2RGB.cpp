#include "Gray2RGB.h"

Gray2RGB::Gray2RGB()
{
	this->m_policy = ColorWindowPolicy::proportion;
	this->m_min = 0.01;
	this->m_max = 0.99;
}

Gray2RGB::Gray2RGB(ImageType::Pointer image)
{
	this->m_image = image;
	Gray2RGB();
}

Gray2RGB::~Gray2RGB()
{
}

int Gray2RGB::SetImage(ImageType::Pointer image)
{
	// Check whether input image is null.
	if (image.IsNull())
	{
		std::cerr << "**Error setting an empty image to Gray2RGB class." << std::endl;
		return 1;
	}
	
	this->m_image = image;
	
	return 0;
}

int Gray2RGB::SetColormap(ColormapType colormap)
{
	// If color map contains less than two key-value pairs, report an error.
	if (colormap.size() < 2)
	{
		std::cerr << "**Error: colormap should contains at least two colors." << std::endl;
		return 1;
	}
	
	this->m_colormap.swap(colormap);
	return 0;
}

void Gray2RGB::SetColorWindowPolicyToAll(void)
{
	this->m_policy = ColorWindowPolicy::all;
	this->m_min = 0;
	this->m_max = 1;
}

void Gray2RGB::SetColorWindowPolicyToRange(double min, double max)
{
	// If min is larger than max, swap them.
	if (min > max)
		std::swap(min, max);
	
	this->m_policy = ColorWindowPolicy::range;
	this->m_min = min;
	this->m_max = max;
}

void Gray2RGB::SetColorWindowPolicyToProportion(double min_p, double max_p)
{
	// If min_p is larger than max_p, swap them.
	if (min_p > max_p)
		std::swap(min_p, max_p);
	
	this->m_policy = ColorWindowPolicy::proportion;
	this->m_min = min_p < 0 ? 0 : min_p;
	this->m_max = max_p > 1 ? 1 : max_p;
}

int Gray2RGB::GetMinMaxPixelValues(double& min, double& max)
{
	// Check whether image has been set.
	if (this->m_image.IsNull())
	{
		std::cerr << "**Error getting minimal and maximal values from null image." << std::endl;
		return 1;
	}
	
	// Compute.
	typedef itk::MinimumMaximumImageCalculator< ImageType > MinMaxCalculatorType;
	MinMaxCalculatorType::Pointer mmCalculator = MinMaxCalculatorType::New();
	mmCalculator->SetImage(this->m_image);
	try
	{
		mmCalculator->Compute();
	}
	catch (itk::ExceptionObject &err)
	{
		std::cerr << "**Error getting minimal and maximal values." << std::endl;
		std::cerr << err;
		return 1;
	}
	min = mmCalculator->GetMinimum();
	max = mmCalculator->GetMaximum();
	return 0;
}

int Gray2RGB::GetMinMaxPixelValuesByProportion(double min_p, double max_p, double& min, double& max)
{
	// Check whether image has been set.
	if (this->m_image.IsNull())
	{
		std::cerr << "**Error accessing pixel values from null image." << std::endl;
		return 1;
	}
	
	// Sort image pixels.
	ImageType::RegionType region = this->m_image->GetLargestPossibleRegion();
	size_t size = region.GetSize(0) * region.GetSize(1) * region.GetSize(2);
	std::vector< double > pixels(reinterpret_cast<double*>(this->m_image->GetBufferPointer()), \
		reinterpret_cast<double*>(this->m_image->GetBufferPointer()) + size);
	std::sort(pixels.begin(), pixels.end());
	
	// Get min & max.
	min = pixels.at(std::ceil(min_p * size));
	max = pixels.at(std::floor(max_p * size));
	if (min > max)
		std::swap(min, max);
	return 0;
}

int Gray2RGB::Convert2ColorWindowPosition(double min, double max, ImageType::Pointer& posImage)
{
	posImage = NULL;
	
	// If min equals to max, report an fetal error.
	if (min == max)
	{
		std::cerr << "**Fetal Error: same minimal and maximal values passed to Covert2ColorWindowPosition function. It will cause division by zero." << std::endl;
		return 1;
	}
	
	// Check whether image is null.
	if (this->m_image.IsNull())
	{
		std::cerr << "**Error converting to color window position with null image." << std::endl;
		return 1;
	}
	
	// Swap min and max if min is larger than max.
	if (min > max)
		std::swap(min, max);
	
	// Subtract by min.
	typedef itk::SubtractImageFilter< ImageType > SubtractFilterType;
	SubtractFilterType::Pointer subtractFilter = SubtractFilterType::New();
	subtractFilter->SetInput1(this->m_image);
	subtractFilter->SetConstant2(min);
	try
	{
		subtractFilter->Update();
	}
	catch (itk::ExceptionObject &err)
	{
		std::cerr << "**Error: fail to covert to color window position when subtracting min value." << std::endl;
		std::cerr << err;
		return 1;
	}
	
	// Divide by (max - min).
	typedef itk::DivideImageFilter< ImageType, ImageType, ImageType > DivideFilterType;
	DivideFilterType::Pointer divideFilter = DivideFilterType::New();
	divideFilter->SetInput1(subtractFilter->GetOutput());
	divideFilter->SetConstant2(max - min);
	try
	{
		divideFilter->Update();
	}
	catch (itk::ExceptionObject &err)
	{
		std::cerr << "**Error: fail to covert to color window position when dividing by (max - min)." << std::endl;
		std::cerr << err;
		return 1;
	}
	
	posImage = divideFilter->GetOutput();
	return 0;
}

int Gray2RGB::Convert2RGB(void)
{
	this->m_rgbImage = NULL;
	
	// Check whether image has been set.
	if (this->m_image.IsNull())
	{
		std::cerr << "**Error converting null image to RGB." << std::endl;
		return 1;
	}
	
	// Check whether color map has been set.
	if (this->m_colormap.size() < 2)
	{
		std::cerr << "**Error converting to RGB without a legal colormap." << std::endl;
		return 1;
	}
	
	// Detect range of pixels to be mapped.
	double min, max;
	if (this->m_policy == ColorWindowPolicy::all)
	{
		if (this->GetMinMaxPixelValues(min, max) != 0)
			return 1;
		std::cout << "All pixel values of input image are mapped to RGB colors. The range is [" << min << "," << max << "]." << std::endl;
	}
	else if (this->m_policy == ColorWindowPolicy::proportion)
	{
		if (this->GetMinMaxPixelValuesByProportion(this->m_min, this->m_max, min, max))
			return 1;
		std::cout << "Tailed pixels are abandon when mapping to RGB colors. Proportion range is [" << this->m_min << "," << this->m_max << "]. ";
		std::cout << "Pixel range is [" << min << "," << max << "]." << std::endl;
	}
	else //if (this->m_policy == ColorWindowPolicy::range)
	{
		min = this->m_min;
		max = this->m_max;
		std::cout << "Tailed pixels are abandon when mapping to RGB colors. The range is [" << min << "," << max << "]. " << std::endl;
	}
	
	// If min equals to max, report a fetal error.
	if (min == max)
	{
		std::cerr << "**Fetal Error: range of pixels to be mapped has same lower and upper bound. Possiboly due to plane image or bad specified range." << std::endl;
		return 1;
	}
	
	// Convert to color window position.
	ImageType::Pointer posImage;
	if (this->Convert2ColorWindowPosition(min, max, posImage) != 0)
		return 1;
	
	// Iterate posImage to build up RGB image.
	this->m_rgbImage = RGBImageType::New();
	this->m_rgbImage->SetOrigin(posImage->GetOrigin());
	this->m_rgbImage->SetSpacing(posImage->GetSpacing());
	this->m_rgbImage->SetDirection(posImage->GetDirection());
	this->m_rgbImage->SetRegions(posImage->GetLargestPossibleRegion());
	this->m_rgbImage->Allocate();
	
	itk::ImageRegionConstIterator< ImageType > imageIter(posImage, posImage->GetLargestPossibleRegion());
	while (!imageIter.IsAtEnd())
	{
		double pos = imageIter.Get();
		if (pos <= this->m_colormap.begin()->first)
		{
			this->m_rgbImage->SetPixel(imageIter.GetIndex(), this->Tuple2RGB(this->m_colormap.begin()->second));
		}
		else if (pos >= this->m_colormap.rbegin()->first)
		{
			this->m_rgbImage->SetPixel(imageIter.GetIndex(), this->Tuple2RGB(this->m_colormap.rbegin()->second));
		}
		else
		{
			// Find an iter whose key is larger than current position.
			auto iter = this->m_colormap.begin();
			while (iter->first < pos)
				++iter;
			
			// Previous iter.
			auto preIter = iter;
			--preIter;
			
			// Interpolation weight.
			double w = (pos - preIter->first) / (iter->first - preIter->first);
			
			this->m_rgbImage->SetPixel(imageIter.GetIndex(), this->InterpolateTuples2RGB(preIter->second, iter->second, w));
			
		}
		++imageIter;
	}
	
	return 0;
}
