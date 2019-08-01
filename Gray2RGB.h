/* 
 * File:          Gray2RGB.h
 * Description:   To paint image as specified color map.
 * Author:        Zhang Teng, PhD
 * Organization:  Zhejiang University Medical PET Center
 * Mailbox:       zhangteng630@zju.edu.cn
 * License:       GPL-3.0, http://www.gnu.org/licenses/gpl-3.0.en.html
 * 
 * Version 1.0, Created on July 31, 2019, 2:10 PM
 */

#ifndef __Gray2RGB_h__
#define __Gray2RGB_h__

#include <map>
#include <tuple>
#include <vector>
#include <algorithm>

#include <itkImage.h>
#include <itkRGBPixel.h>
#include <itkMinimumMaximumImageCalculator.h>
#include <itkSubtractImageFilter.h>
#include <itkDivideImageFilter.h>
#include <itkImageRegionConstIterator.h>

class Gray2RGB {
public:
	/** Define types. **/
	// Input image type: images whose pixel types are not double should be cast to this image type.
	typedef itk::Image< double, 3 > ImageType;
	
	// RGB pixel and RGB image type.
	typedef itk::RGBPixel< unsigned char > RGBPixel;
	typedef itk::Image< RGBPixel, 3 > RGBImageType;
	
	// Color map type: key is value between 0 and 1 (not forced, values outside 0 and 1 will extend color window), indicating position at color window.
	typedef std::map< double, std::tuple< unsigned char, unsigned char, unsigned char > > ColormapType;
	
	// Color window policy.
	enum ColorWindowPolicy {
		all,           // Map all pixels.
		range,         // Map pixels within range [min, max].
		proportion,    // Map pixels within range [min, max] where min and max are min_p and max_p smallest pixel values.
	};
	
public:
	Gray2RGB();
	Gray2RGB(ImageType::Pointer image);
	virtual ~Gray2RGB();
	
	// Set input image.
	int SetImage(ImageType::Pointer image);
	
	// Set color map.
	int SetColormap(ColormapType colormap);
	
	// Set color window policy. Default equals to SetColorWindowPolicyToProportion(0.01, 0.99).
	void SetColorWindowPolicyToAll(void);                                  // Use 'all' policy.
	void SetColorWindowPolicyToRange(double min, double max);              // Use 'range' policy.
	void SetColorWindowPolicyToProportion(double min_p, double max_p);     // Use 'proportion' policy. If min_p/max_p is less/larger than 0, it will be forced to 0/1.
	
	// Convert to RGB.
	int Convert2RGB(void);
	int Update(void) {
		return this->Convert2RGB();
	}
	
	// Get RGB image.
	RGBImageType::Pointer& GetRGBImage(void) {
		return this->m_rgbImage;
	}
	
private:
	// Get min and max pixel values.
	int GetMinMaxPixelValues(double& min, double& max);
	
	// Sort pixel values and get min/max as the min_p/max_p smallest pixel value.
	int GetMinMaxPixelValuesByProportion(double min_p, double max_p, double& min, double& max);
	
	// Convert pixel values to position at color window : p = (x-min)/(max-min).
	int Convert2ColorWindowPosition(double min, double max, ImageType::Pointer& posImage);
	
	// Convert tuple to RGB pixel.
	inline RGBPixel Tuple2RGB(std::tuple< unsigned char, unsigned char, unsigned char > tuple) {
		RGBPixel rgb;
		rgb.Set(std::get<0>(tuple), std::get<1>(tuple), std::get<2>(tuple));
		return rgb;
	}
	
	// Interpolate two tuples to a RGB pixel. Return (1-w)*tuple0 + w*tuple1.
	inline RGBPixel InterpolateTuples2RGB(std::tuple< unsigned char, unsigned char, unsigned char > tuple0,\
		std::tuple< unsigned char, unsigned char, unsigned char > tuple1, double w) {
		RGBPixel rgb;
		rgb.Set(std::round((1.0 - w) * std::get<0>(tuple0) + w * std::get<0>(tuple1)),\
			std::round((1.0 - w) * std::get<1>(tuple0) + w * std::get<1>(tuple1)),\
			std::round((1.0 - w) * std::get<2>(tuple0) + w * std::get<2>(tuple1)));
		return rgb;
	}
	
private:
	// Input image.
	ImageType::Pointer m_image;
	
	// RGB image.
	RGBImageType::Pointer m_rgbImage;
	
	// Color map.
	ColormapType m_colormap;
	
	// Color window policy and additional min/max (proportion) values.
	ColorWindowPolicy m_policy;
	double m_min, m_max;
};

#endif /* __Gray2RGB_h__ */

