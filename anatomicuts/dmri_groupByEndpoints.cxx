/* Andrew Zhang
 * Professor Siless
 * dmri_groupingByEndpoints.cxx
 *
 */

#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <vector>
#include <algorithm>

#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageRegionConstIterator.h>

#include "itkMesh.h"
#include <vtkPolyData.h>
#include <vtkPolyDataReader.h>
#include <vtkPolyDataWriter.h>
#include "itkPolylineCell.h"
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <cmath>
#include "itkPolylineCell.h"
#include "GetPot.h"
#include "TrkVTKPolyDataFilter.txx"
#include "PolylineMeshToVTKPolyDataFilter.h"
#include "LabelPerPointVariableLengthVector.h"
#include "EuclideanMembershipFunction.h"
#include "ClusterTools.h"
#include "itkDefaultStaticMeshTraits.h"

using namespace std;

int main(int narg, char* arg[]) 
{
	//Take in inputs
	GetPot c1(narg, const_cast<char**>(arg));

	//Usage error
	if (c1.size() == 1 || c1.search(2, "--help", "-h"))
	{
		cout << "Usage: " << endl; 
		cout << arg[0] << " -s streamlines -i imageFile -d outputDirectory" << endl;
		return -1; 
	}

	
	//Take in information
//	const char *stream_lines = c1.follow("string_file.trk", "-s"); 
	const char *image_file = c1.follow("image_file.nii.gz", "-i"); 
	const char *output = c1.follow("output_directory", "-d"); 

	//Read in files
	vector<string> inputFiles; 
	for (string inputName = string(c1.follow("", 2, "-s", "-S")); access(inputName.c_str(), 0) == 0; inputName = string(c1.next("")))
	{
		inputFiles.push_back(inputName); 
	}

	//Variable definitions
	enum {Dimension =3};
	typedef float                                                        PixelType;
	const unsigned int PointDimension = 3;
	typedef std::vector<int>                  PointDataType;
	const unsigned int MaxTopologicalDimension = 3;
	typedef double CoordinateType;
	typedef double InterpolationWeightType;
	typedef itk::DefaultStaticMeshTraits<
		PointDataType, PointDimension, MaxTopologicalDimension,
		CoordinateType, InterpolationWeightType, PointDataType > MeshTraits;
	typedef itk::Mesh< PixelType, PointDimension, MeshTraits > HistogramMeshType;

	typedef itk::Image<float, 3> ImageType;
	
	typedef itk::Mesh< PixelType, PointDimension > ColorMeshType;
	typedef ColorMeshType::PointType PointType;
	typedef ColorMeshType::CellType        CellType;
	typedef itk::PolylineCell<CellType>                      PolylineCellType;
	typedef ColorMeshType::CellAutoPointer CellAutoPointer;

	vector<ColorMeshType::Pointer>* meshes; 
	vector<vtkSmartPointer<vtkPolyData>> polydatas; 
	ImageType::Pointer inputImage;  

	typedef ClusterTools<ColorMeshType, ImageType, HistogramMeshType> ClusterToolsType; 
	ClusterToolsType::Pointer clusterTools = ClusterToolsType::New(); 

	clusterTools->GetPolyDatas(inputFiles, &polydatas, inputImage); 
	
	//Take in input trk file
	meshes = clusterTools->PolydataToMesh(polydatas); 

	ColorMeshType::Pointer input = (*meshes)[0];
	int pointIndices = 0; 
	int cellIndices = 0; 
	ColorMeshType::CellsContainer::Iterator  inputCellIt = input->GetCells()->Begin(); 

	//Keep all the streamlines with the same start and end locations in a vector
	vector<ColorMeshType::CellsContainer::Iterator> streamlines; 
	//Store the values that the wanted streamlines lie within
	vector<int> image_values; 

	cerr << "New mesh: " << endl; 
	int stream_count = 0; 

	using ImageType = Image<PixelType, Dimension>; 
	using ReaderType = ImageFileReader<ImageType>; 

	ReaderType::Pointer reader = ReaderType::New(); 

	reader->SetFileName(image_file); 
	reader->Update();
			
	inputImage = reader->GetOutput(); 

	ImageType::IndexType index1, index2; 

	//Cycles through each streamline
	for (int cellId = 0; inputCellIt != input->GetCells()->End(); ++inputCellIt, cellId++)
	{
		cerr << cellId << endl; 

		PointType firstPt, start, second_check, end; 
		firstPt.Fill(0); 

		CellType::PointIdIterator it = inputCellIt.Value()->PointIdsBegin(); 
		input->GetPoint(*it, &firstPt); 
		double lengthSoFar = 0; 

		start = firstPt; 

		cerr << "First point: " << start << endl; 

		//Goes through each point in a streamline
		//Converted given for loop into a while loop
		while (it != inputCellIt.Value()->PointIdsEnd())
		{
			PointType pt; 
			pt.Fill(0);
			input->GetPoint(*it, &pt);
			lengthSoFar += firstPt.EuclideanDistanceTo(pt); 
			input->GetPoint(*it, &firstPt); 

			it++; 
				
			//Store second to last point in case endpoint gives value 0
			if (it == inputCellIt.Value()->PointIdsEnd())
				end = pt; 
			else
				second_check = pt; 
		}  
			
		//Value of coordinates based on image
		float value1 = 0, value2 = 0; 
	
		if (inputImage->TransformPhysicalPointToIndex(start, index1)) 
		{
			value1 = inputImage->GetPixel(index1); 
			cerr << index1 << " = " << value1 << endl; 
		}

		if (inputImage->TransformPhysicalPointToIndex(end, index2)) 
		{
			value2 = inputImage->GetPixel(index2);
			
			if (value2 == 0)
			{
				if (inputImage->TransformPhysicalPointToIndex(second_check, index2))
					value2 = inputImage->GetPixel(index2);
			}

			cerr << index2 << " = " << value2 << endl; 
		}

		//Test trk file from output?		
/*
		index1[0] = start[0];
		index1[1] = start[1];
		index1[2] = start[2];
		index2[0] = end[0];
		index2[1] = end[1];
		index2[2] = end[2];

		cerr << index1[0] << " " << index2[0] << endl; 

		value1 = inputImage->GetPixel(index1);
		value2 = inputImage->GetPixel(index2); 
*/

		if (value1 != 0 and value1 == value2)
		{
			cout << "Start and ends match: " << endl; 
			cout << "First point: " << index1 << endl; 
			cout << "Last point: " << index2 << endl; 
			cout << index1 << " = " << value1 << endl; 
			cout << index2 << " = " << value2 << endl;  
			stream_count++; 
			streamlines.push_back(inputCellIt); 
			image_values.push_back(value1); 
		}	
	}

	//Store streamlines that lie in the same structure, then place into one trk file

	//Get rid of duplicates
	vector<ColorMeshType::CellsContainer::Iterator> filtered_streamlines; 	
	vector<int> filtered_values; 

	sort(image_values.begin(), image_values.end()); 

	//Find a way to change the order of the streamlines as well
	//Keep them unorganized, check previous values if they've been used?


	int compare = image_values[0]; 
	filtered_values.push_back(image_values[0]); 
	filtered_streamlines.push_back(streamlines[0]); 
	for (int i = 1; i < image_values.size(); i++) 
	{
		if (image_values[i] != compare)
		{
			filtered_values.push_back(image_values[i]); 
			filtered_streamlines.push_back(streamlines[i]); 
			compare = image_values[i]; 
		}
	}


	cerr << "Total of " << stream_count << " streamlines" << endl; 
	cerr << "Image values: "; 
	for (int i = 0; i < filtered_values.size(); i++) 
	{
		cerr << filtered_values[i] << " "; 
	}
	cerr << endl; 

	cerr << "Number of values: " << filtered_values.size() << endl; 
	cerr << "Number of streamlines: " << filtered_streamlines.size() << endl; 

	//Output files
	ColorMeshType::Pointer om = ColorMeshType::New(); 
	om->SetCellsAllocationMethod(ColorMeshType::CellsAllocatedDynamicallyCellByCell); 

	//Needs to loop based on how many time each structure appears
	for (int i = 0; i < streamlines.size(); i++)
	//for (int i = streamlines.size() - 1; i > -1; i--)
	{
		CellAutoPointer line; 
		line.TakeOwnership(new PolylineCellType); 
		int k = 0; 
		CellType::PointIdIterator it = streamlines[i].Value()->PointIdsBegin(); 
		for (; it != streamlines[i].Value()->PointIdsEnd(); it++)
		{
			PointType pt; 
			input->GetPoint(*it, &pt);

			om->SetPoint(pointIndices, pt); 
			line->SetPointId(k, pointIndices); 

			k++; 
			pointIndices++; 
		}	
		om->SetCell(cellIndices, line); 
		ColorMeshType::CellPixelType cellData; 
		input->GetCellData(i, &cellData); 
		om->SetCellData(cellIndices, cellData); 
		cellIndices++; 	
		
		string outputName; 
		string number = to_string(filtered_values[i]); 
		//string filename = inputFiles[0].substr(inputFiles[0].find_last_of("/\\") + 1);
		string filename = number + ".trk"; 
		outputName = string(output) + "/" + filename; 

		cout << "Mesh name: " << outputName << endl; 

		clusterTools->SaveMesh(om, inputImage, outputName, inputFiles[0]); 
	}

	delete meshes;

	return 0; 	
}


//Check points before endpoint if endpoint gives value zero
//What if starts at 0?

/*
 * Ushaped fibers go into a TRK file
 * Check endpoints and put into same region TRK file
 * If they connect two different structures, ignore
 *
 * First open/read files
 * Then identify endpoints
 * Then check if in same region
 * if so, then output them to TRK file and into a folder
 *
 */

/*	//Some variabel names are repeated but have _img added
	using PixelType_img = unsigned char; 
	constexpr unsigned int Dimension_img = 2; 

	using ImageType = Image<PixelType_img, Dimension_img>;
	using PointSetType = PointSet<PixelType_img, Dimension_img>; 
	using ReaderType = ImageFileReader<ImageType>; 

	ReaderType::Pointer reader = ReaderType::New(); 

	const char* inputFilename = arg[1]; //Test if image is being read 
	reader->SetFileName(inputFilename); 

	try
	{
		reader->Update(); 
	}
	catch(ExceptionObject &err)
	{
		cout << "ExceptionObject caught !" << endl; 
		cout << err << endl; 
		return EXIT_FAILURE; 
	}
*/
	//Extract pixels from image
/*	using ImageType = Image<unsigned short, 3>; 

	ImageType::Pointer image = ImageType::New(); 

	const ImageType::SizeType size = {{200, 200, 200}}; 
	const ImageType::IndexType start = {{0, 0, 0}}; 

	ImageType::RegionType region; 
	region.SetSize(size); 
	region.SetIndex(start); 

	image->SetRegions(region); 
	image->Allocate(true); 

	const ImageType::IndexType pixelIndex = {{144, 168, 75}}; 

	ImageType::PixelType pixelValue = image->GetPixel(pixelIndex); 

	image->SetPixel(pixelIndex, pixelValue); 
*/
/*
			inputImage->TransformPhysicalPointToIndex(start, index1);	
			inputImage->TransformPhysicalPointToIndex(end, index2); 
 
			cout << "First point: " << index1 << endl; 
			cout << "Last point: " << index2 << endl; 
			
			float value1 = inputImage->GetPixel(index1); 
			float value2 = inputImage->GetPixel(index2); 
			//const PixelType value1 = inputImage->GetPixel(index1); 
			//const PixelType value2 = inputImage->GetPixel(index2); 

			cout << index1 << " = " << value1 << endl; 
			cout << index2 << " = " << value2 << endl; 
 	
			if (value1 == value2)
			{
				cout << "First point: " << index1 << endl; 
				cout << "Last point: " << index2 << endl; 
				cout << index1 << " = " << value1 << endl; 
				cout << index2 << " = " << value2 << endl; 
			}		
*/
