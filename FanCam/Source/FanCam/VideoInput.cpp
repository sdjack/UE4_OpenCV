// Author: Steven Jackson

#include "VideoInput.h"
#include "Tracking/FacialRecognition.h"
#include <iostream>

// Sets default values
AVideoInput::AVideoInput()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Initialize OpenCV and webcam properties
	isStreamOpen = false;
	ShouldResize = false;
	CameraID = 0;
	OperationMode = 0;
	MaxVertices = 1000;
	RefreshRate = 30.0f;
	RefreshTimer = 0.0f;
	VertexFoundCount = 0;
	VideoSizeVector = FVector2D(0.0f, 0.0f);
	VideoResizeVector = FVector2D(320.0f, 240.0f);

	VideoStream = cv::VideoCapture();
	VideoFrame = cv::Mat();
	_algorithm = NULL;

}

AVideoInput::~AVideoInput()
{
}

// Called when the game starts or when spawned
void AVideoInput::BeginPlay()
{
	Super::BeginPlay();

	// Open the stream
	VideoStream.open(CameraID);
	if (VideoStream.isOpened())
	{
		// Initialize stream
		isStreamOpen = true;
		UpdateFrame();
		VideoSizeVector = FVector2D(VideoFrame.cols, VideoFrame.rows);
		VideoSize = cv::Size(VideoResizeVector.X, VideoResizeVector.Y);
		VideoTexture = UTexture2D::CreateTransient(VideoSizeVector.X, VideoSizeVector.Y);
		VideoTexture->UpdateResource();
		VideoUpdateTextureRegion = new FUpdateTextureRegion2D(0, 0, 0, 0, VideoSizeVector.X, VideoSizeVector.Y);
		// Initialize data array
		VideoData.Init(FColor(0, 0, 0, 255), VideoSizeVector.X * VideoSizeVector.Y);
		Vertices.Init(FVector2D(0, 0), MaxVertices);

		if (!_algorithm) {
			_algorithm = VisionAlgorithm::NewWorker(VideoFrame, Vertices, VertexFoundCount, MaxVertices);
		}

		// Do first frame
		DoProcessing();
		UpdateTexture();
		OnNextVideoFrame();
	}
}

// Called every frame
void AVideoInput::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RefreshTimer += DeltaTime;
	if (isStreamOpen && RefreshTimer >= 1.0f / RefreshRate)
	{
		RefreshTimer -= 1.0f / RefreshRate;
		UpdateFrame();
		DoProcessing();
		UpdateTexture();
		OnNextVideoFrame();
	}
}

void AVideoInput::ChangeOperation()
{
	OperationMode++;
	OperationMode %= 3;
}

void AVideoInput::UpdateFrame()
{
	if (VideoStream.isOpened())
	{
		VideoStream.read(VideoFrame);
		if (ShouldResize)
		{
			cv::resize(VideoFrame, VideoFrame, VideoSize);
		}
	}
	else {
		isStreamOpen = false;
	}
}

void AVideoInput::DoProcessing()
{
	if (OperationMode == 0) {
		// Mode 0
	}
	else if (OperationMode == 1) {
		// Mode 1
	}
	else if (OperationMode == 2) {
		// Mode 2
	}

}

void AVideoInput::UpdateTexture()
{
	if (isStreamOpen && VideoFrame.data)
	{
		// Update Mat data with changes from tracking algorithm
		for (int i = 0; i < VertexFoundCount; ++i) {
			cv::Point vertex = cv::Point(Vertices[i].X, Vertices[i].Y);
			cv::drawMarker(VideoFrame, vertex, cv::Scalar(0, 255, 0));
		}
		// Copy Mat data to Data array
		for (int y = 0; y < VideoSizeVector.Y; y++)
		{
			for (int x = 0; x < VideoSizeVector.X; x++)
			{
				int i = x + (y * VideoSizeVector.X);
				VideoData[i].B = VideoFrame.data[i * 3 + 0];
				VideoData[i].G = VideoFrame.data[i * 3 + 1];
				VideoData[i].R = VideoFrame.data[i * 3 + 2];
			}
		}

		uint8* srcData = (uint8*)VideoData.GetData();

		// Update texture 2D
		UpdateTextureRegions(VideoTexture, VideoUpdateTextureRegion, (uint32)(4 * VideoSizeVector.X), srcData, false);
	}
}

void AVideoInput::UpdateTextureRegions(UTexture2D* texture, FUpdateTextureRegion2D* regions, uint32 srcpitch, uint8* srcData, bool freedata)
{
	if (texture->Resource)
	{
		struct FUpdateTextureRegionsData
		{
			FTexture2DResource* Texture2DResource;
			int32 MipIndex;
			uint32 NumRegions;
			FUpdateTextureRegion2D* Regions;
			uint32 SrcPitch;
			uint32 SrcBpp;
			uint8* SrcData;
		};

		FUpdateTextureRegionsData* RegionData = new FUpdateTextureRegionsData;

		RegionData->Texture2DResource = (FTexture2DResource*)texture->Resource;
		RegionData->MipIndex = (int32)0;
		RegionData->NumRegions = (uint32)1;
		RegionData->Regions = regions;
		RegionData->SrcPitch = srcpitch;
		RegionData->SrcBpp = (uint32)4;
		RegionData->SrcData = srcData;

		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
			UpdateTextureRegionsData,
			FUpdateTextureRegionsData*, RegionData, RegionData,
			bool, freedata, freedata,
			{
			for (uint32 RegionIndex = 0; RegionIndex < RegionData->NumRegions; ++RegionIndex)
			{
				int32 CurrentFirstMip = RegionData->Texture2DResource->GetCurrentFirstMip();
				if (RegionData->MipIndex >= CurrentFirstMip)
				{
					RHIUpdateTexture2D(
						RegionData->Texture2DResource->GetTexture2DRHI(),
						RegionData->MipIndex - CurrentFirstMip,
						RegionData->Regions[RegionIndex],
						RegionData->SrcPitch,
						RegionData->SrcData
						+ RegionData->Regions[RegionIndex].SrcY * RegionData->SrcPitch
						+ RegionData->Regions[RegionIndex].SrcX * RegionData->SrcBpp
						);
				}
			}
			if (freedata)
			{
				FMemory::Free(RegionData->Regions);
				FMemory::Free(RegionData->SrcData);
			}
			delete RegionData;
			});
	}
}

