/* 
 * File:          Gray2Rainbow.cxx
 * Description:   To pain rainbow colors for input PET image.
 * Author:        Zhang Teng, PhD
 * Organization:  Zhejiang University Medical PET Center
 * Mailbox:       zhangteng630@zju.edu.cn
 * License:       GPL-3.0, http://www.gnu.org/licenses/gpl-3.0.en.html
 * 
 * Version 1.0, Created on July 31, 2019, 4:56 PM
 */

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>

#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>

#include "Gray2RGB.h"

void PrintUsage(char * exe)
{
	fprintf(stderr, "\nGray2Rainbow program Version 1.0, written by Zhang Teng, Ph.D. @ Zhejiang University PET Center.\n\n");
	fprintf(stderr, "Usage   :  %s inputImage outputRGBImage (options)\n", exe);
	fprintf(stderr, "Options :\n");
	fprintf(stderr, "           -all : mapping all pixels values.\n");
	fprintf(stderr, "           -range min max : mapping pixels within range [min, max].\n");
	fprintf(stderr, "           -proportion min_p max_p : detect min and max pixel values that larger than proportion of all pixels.\n");
}

int main(int argc, char * argv[])
{	
	if (argc < 3)
	{
		PrintUsage(argv[0]);
		return 0;
	}
	
	std::string imageFileName(argv[1]);
	std::string rgbImageFileName(argv[2]);	
	
	Gray2RGB gray2rgb;
	
	// Rainbow colormap
	Gray2RGB::ColormapType colormap;
	colormap[0.0000] = std::make_tuple< unsigned char, unsigned char, unsigned char >(0, 0, 0);
	colormap[0.1255] = std::make_tuple< unsigned char, unsigned char, unsigned char >(64, 0, 128);
	colormap[0.2510] = std::make_tuple< unsigned char, unsigned char, unsigned char >(0, 0, 255);
	colormap[0.3765] = std::make_tuple< unsigned char, unsigned char, unsigned char >(0, 255, 0);
	colormap[0.6275] = std::make_tuple< unsigned char, unsigned char, unsigned char >(255, 255, 0);
	colormap[1.0000] = std::make_tuple< unsigned char, unsigned char, unsigned char >(255, 0, 0);
	if (gray2rgb.SetColormap(colormap) != 0)
		return 1;
	
	// Read image.
	typedef Gray2RGB::ImageType ImageType;
	typedef itk::ImageFileReader< ImageType > ImageReaderType;
	ImageReaderType::Pointer imageReader = ImageReaderType::New();
	imageReader->SetFileName(imageFileName);
	try
	{
		imageReader->Update();
	}
	catch (itk::ExceptionObject &err)
	{
		std::cerr << "**Error reading image " << imageReader->GetFileName() << std::endl;
		std::cerr << err;
		return 1;
	}
	if (gray2rgb.SetImage(imageReader->GetOutput()) != 0)
		return 1;
	
	// Get options (if any) and set policy.	
	if (argc > 3)
	{
		if (std::strcmp(argv[3], "-all") == 0)
		{
			gray2rgb.SetColorWindowPolicyToAll();
		}
		else if (std::strcmp(argv[3], "-range") == 0)
		{
			if (argc < 6)
			{
				std::cerr << "**Error: range boundaries not specified." << std::endl;
				return 1;
			}
			gray2rgb.SetColorWindowPolicyToRange(std::atof(argv[4]), std::atof(argv[5]));
		}
		else if (std::strcmp(argv[3], "-proportion") == 0)
		{
			if (argc < 6)
			{
				std::cerr << "**Error: proportion boundaries not specified." << std::endl;
				return 1;
			}
			gray2rgb.SetColorWindowPolicyToProportion(std::atof(argv[4]), std::atof(argv[5]));
		}
		else 
		{
			std::cerr << "**Error: unresolved option " << argv[3] << std::endl;
			PrintUsage(argv[0]);
			return 1;
		}
	}
	
	// Convert.
	if (gray2rgb.Update() != 0)
		return 1;
	
	// Write.
	typedef itk::ImageFileWriter< Gray2RGB::RGBImageType > RGBImageWriterType;
	RGBImageWriterType::Pointer rgbImageWriter = RGBImageWriterType::New();
	rgbImageWriter->SetInput(gray2rgb.GetRGBImage());
	rgbImageWriter->SetFileName(rgbImageFileName);
	try
	{
		rgbImageWriter->Update();
	}
	catch (itk::ExceptionObject &err)
	{
		std::cerr << "**Error writing RGB image to " << rgbImageWriter->GetFileName() << std::endl;
		std::cerr << err;
		return 1;
	}
	
	return 0;
}
