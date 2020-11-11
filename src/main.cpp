/*=========================================================================

Program: Elastix test
Language: C++
Date: $Date: 2020-09-10 $
Version: $Revision: 1 $
Author: $Ricardo A Corredor J$

==========================================================================*/


#include <iostream>
#include <iomanip>

#include <metaCommand.h>
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkRoundImageFilter.h"
#include "itkImage.h"
#include "itkCastImageFilter.h"

#include "itkParameterMapInterface.h"
#include "itkParameterFileParser.h"
#include "elastixlib.h"


int main(int argc, char *argv[])
{


	std::string movingImgFilePath, fixedImgFilePath, elastixParamsFilePath, out_1_DirPath, out_2_DirPath;
	movingImgFilePath = "..\\..\\elastix-cpp-test\\data\\image_moving.nii.gz";
	fixedImgFilePath = "..\\..\\elastix-cpp-test\\data\\image_fixed.nii.gz";
	out_1_DirPath = "..\\..\\elastix-cpp-test\\data\\out_1";
	out_2_DirPath = "..\\..\\elastix-cpp-test\\data\\out_2";
	elastixParamsFilePath = "..\\..\\elastix-cpp-test\\params\\params.txt";

	bool activateLogs = true, activateCout = false, resetImgOriginAndDir = true, roundImgSpc = true;
	

	///////////////////////////////////////////////////////////////////////////////////////////////

	const unsigned int dimension = 3;
	typedef float InputPixelType;
	typedef float OutputPixelType;
	typedef int IntPixelType;
	typedef itk::Image<InputPixelType, dimension> InputImageType;
	typedef itk::Image<OutputPixelType, dimension> OutputImageType;
	typedef itk::Image<IntPixelType, dimension> IntImageType;
	typedef itk::ImageFileReader<InputImageType> ReaderType;
	typedef itk::ImageFileWriter<OutputImageType> FloatWriterType;
	typedef itk::ImageFileWriter<IntImageType> UShortWriterType;
	typedef itk::RoundImageFilter<OutputImageType, OutputImageType> RoundFilterType;
	typedef itk::ParameterFileParser ParameterFileParserType;
	typedef itk::CastImageFilter<OutputImageType, IntImageType> CastFloatToIntFilterType;

	ReaderType::Pointer readerFixedImg = ReaderType::New();
	readerFixedImg->SetFileName(fixedImgFilePath);
	readerFixedImg->Update();
	InputImageType::Pointer fixed_img = readerFixedImg->GetOutput();

	ReaderType::Pointer readerMovingImg = ReaderType::New();
	readerMovingImg->SetFileName(movingImgFilePath);
	readerMovingImg->Update();
	InputImageType::Pointer moving_img = readerMovingImg->GetOutput();

	InputImageType::SpacingType inFixedImgSpc = fixed_img->GetSpacing();
	InputImageType::SpacingType inMovingImgSpc = moving_img->GetSpacing();

	InputImageType::PointType newImgOrigin;
	newImgOrigin[0] = 0;
	newImgOrigin[1] = 0;
	newImgOrigin[2] = 0;

	InputImageType::DirectionType newImgDirection;
	newImgDirection.SetIdentity();
	newImgDirection[0][0] = 0;
	newImgDirection[1][1] = 0;
	newImgDirection[2][2] = 0;
	newImgDirection[0][2] = -1;
	newImgDirection[1][0] = 1;
	newImgDirection[2][1] = -1;

	typedef itk::ChangeInformationImageFilter<InputImageType> ChangeImagePropsFilterType;
	ChangeImagePropsFilterType::Pointer changeMovingImgPropsFilter = ChangeImagePropsFilterType::New();
	ChangeImagePropsFilterType::Pointer changeFixedImgPropsFilter = ChangeImagePropsFilterType::New();

	bool hasChangesMovingImgProps = false, hasChangesFixedImgProps = false;

	if (resetImgOriginAndDir)
	{
		hasChangesMovingImgProps = true;
		hasChangesFixedImgProps = true;
		changeMovingImgPropsFilter->SetOutputDirection(newImgDirection);
		changeMovingImgPropsFilter->SetOutputOrigin(newImgOrigin);
		changeMovingImgPropsFilter->SetChangeOrigin(true);
		changeMovingImgPropsFilter->SetChangeDirection(true);

		changeFixedImgPropsFilter->SetOutputDirection(newImgDirection);
		changeFixedImgPropsFilter->SetOutputOrigin(newImgOrigin);
		changeFixedImgPropsFilter->SetChangeOrigin(true);
		changeFixedImgPropsFilter->SetChangeDirection(true);
	}

	// Apply changes to moving image 
	if (roundImgSpc)
	{
		hasChangesMovingImgProps = true;
		InputImageType::SpacingType newImgSpacing = moving_img->GetSpacing();

		//  Set the spacing of the image 
		newImgSpacing[0] = floor(newImgSpacing[0] * 100.0 + 0.5) / 100.0;
		newImgSpacing[1] = floor(newImgSpacing[1] * 100.0 + 0.5) / 100.0;
		newImgSpacing[2] = floor(newImgSpacing[2] * 100.0 + 0.5) / 100.0;

		changeMovingImgPropsFilter->SetOutputSpacing(newImgSpacing);
		changeMovingImgPropsFilter->SetChangeSpacing(true);
	}

	if (hasChangesMovingImgProps)
	{
		try {
			changeMovingImgPropsFilter->SetInput(moving_img);
			changeMovingImgPropsFilter->Update();
			moving_img = changeMovingImgPropsFilter->GetOutput();
		}
		catch (itk::ExceptionObject exc)
		{
			std::cerr << "Error:" << std::endl;
			std::cerr << exc << std::endl;
		}
	}

	// Apply changes to fixed image 
	if (roundImgSpc)
	{
		hasChangesFixedImgProps = true;
		InputImageType::SpacingType newImgSpacing = fixed_img->GetSpacing();

		//  Set the spacing of the image 
		newImgSpacing[0] = floor(newImgSpacing[0] * 100.0 + 0.5) / 100.0;
		newImgSpacing[1] = floor(newImgSpacing[1] * 100.0 + 0.5) / 100.0;
		newImgSpacing[2] = floor(newImgSpacing[2] * 100.0 + 0.5) / 100.0;

		changeFixedImgPropsFilter->SetOutputSpacing(newImgSpacing);
		changeFixedImgPropsFilter->SetChangeSpacing(true);
	}

	if (hasChangesFixedImgProps)
	{
		try {
			changeFixedImgPropsFilter->SetInput(fixed_img);
			changeFixedImgPropsFilter->Update();
			fixed_img = changeFixedImgPropsFilter->GetOutput();
		}
		catch (itk::ExceptionObject exc)
		{
			std::cerr << "Error:" << std::endl;
			std::cerr << exc << std::endl;
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	elastix::ELASTIX *elastixFilter = new elastix::ELASTIX();

	ParameterFileParserType::Pointer paramsParser = ParameterFileParserType::New();
	paramsParser->SetParameterFileName(elastixParamsFilePath);
	int error = 0;
	try
	{
		paramsParser->ReadParameterFile();
		ParameterFileParserType::ParameterMapType propertiesMap = paramsParser->GetParameterMap();

		error = elastixFilter->RegisterImages(
			static_cast<typename itk::DataObject::Pointer>(fixed_img),
			static_cast<typename itk::DataObject::Pointer>(moving_img),
			propertiesMap,
			out_1_DirPath,
			activateLogs,
			activateCout);
		if (error == 0)
		{
			if (elastixFilter->GetResultImage().IsNotNull())
			{
				OutputImageType::Pointer output_image = static_cast<OutputImageType *>(elastixFilter->GetResultImage().GetPointer());

				std::string outFile = out_1_DirPath + "\\result.nii.gz";

				CastFloatToIntFilterType::Pointer castFilter = CastFloatToIntFilterType::New();
				castFilter->SetInput(output_image);
				castFilter->Update();
				IntImageType::Pointer roundedImg = castFilter->GetOutput();
				
				UShortWriterType::Pointer writerRounded = UShortWriterType::New();
				writerRounded->SetInput(roundedImg);
				writerRounded->SetFileName(outFile);
				writerRounded->Update();
				

				ParameterFileParserType::ParameterMapType transformParamsMap = elastixFilter->GetTransformParameterMap();
				ParameterFileParserType::ParameterValuesType transformationMatrix = transformParamsMap["TransformParameters"];
				
			}
		}
		else
		{
			std::cout << "Error during registration!" << std::endl;
		}

		delete elastixFilter;

		elastix::ELASTIX *elastixFilter2 = new elastix::ELASTIX();
		////////// Second test

		error = elastixFilter2->RegisterImages(
			static_cast<typename itk::DataObject::Pointer>(fixed_img),
			static_cast<typename itk::DataObject::Pointer>(moving_img),
			propertiesMap,
			out_2_DirPath,
			activateLogs,
			activateCout);
		if (error == 0)
		{
			if (elastixFilter2->GetResultImage().IsNotNull())
			{
				OutputImageType::Pointer output_image = static_cast<OutputImageType *>(elastixFilter2->GetResultImage().GetPointer());

				std::string outFile2 = out_2_DirPath + "\\result2.nii.gz";

				CastFloatToIntFilterType::Pointer castFilter2 = CastFloatToIntFilterType::New();
				castFilter2->SetInput(output_image);
				castFilter2->Update();
				IntImageType::Pointer roundedImg2 = castFilter2->GetOutput();

				UShortWriterType::Pointer writerRounded2 = UShortWriterType::New();
				writerRounded2->SetInput(roundedImg2);
				writerRounded2->SetFileName(outFile2);
				writerRounded2->Update();


				ParameterFileParserType::ParameterMapType transformParamsMap = elastixFilter->GetTransformParameterMap();
				ParameterFileParserType::ParameterValuesType transformationMatrix = transformParamsMap["TransformParameters"];

			}
		}
		else
		{
			std::cout << "Error during registration!" << std::endl;
		}



	}
	catch (itk::ExceptionObject & e)
	{
		std::cerr << "Error" << e.what() << std::endl;

	}

	delete elastixFilter;


}