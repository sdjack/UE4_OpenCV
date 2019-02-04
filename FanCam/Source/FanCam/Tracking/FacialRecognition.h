// Author: Steven Jackson

#pragma once

#include "Engine.h"
#include "TrackingRunnable.h"
#include "Runtime/Core/Public/HAL/Runnable.h"
#include "GameFramework/Actor.h"
#include <opencv2/core.hpp>
#include <opencv2/objdetect.hpp>

class FacialRecognition : public TrackingRunnable
{
	// Cascade handlers
	cv::CascadeClassifier FaceCascade;
	cv::CascadeClassifier EyesCascade;

	// The actual tracker processing
	void FindEyes(cv::Mat IN_FaceFrame, const int xCoord, const int yCoord);
	void FindFaces();

public:

	FacialRecognition(cv::Mat& IN_VideoFrame, TArray<FVector2D>& IN_Vertices, int32& IN_VertexFoundCount, const int32 IN_MaxVertices);
	virtual ~FacialRecognition();

	FacialRecognition* getInstance(cv::Mat& IN_VideoFrame, TArray<FVector2D>& IN_Vertices, int32& IN_VertexFoundCount, const int32 IN_MaxVertices);

private:
	// Arbitrary edge-detection params
	const int LOW_THRESHOLD = 50;
	const int RATIO = 3;
	const int KERNEL_SIZE = 3;
	static bool IncludeEyes;

	// For drawing edges
	FColor EDGE_COLOR = FColor::Green;

	// Helper
	static int min(int a, int b) { return a < b ? a : b; }

};


