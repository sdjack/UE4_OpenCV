// Author: Steven Jackson

#pragma once

#include "TrackingRunnable.h"
#include "Engine.h"
#include "Runtime/Core/Public/HAL/Runnable.h"
#include "GameFramework/Actor.h"
#include <opencv2/video/tracking.hpp>
#include <opencv2/core.hpp>
#include <cstdint>


//***********************************************************
//Thread Worker Starts as NULL, prior to being instanced
//		This line is essential! Compiler error without it
TrackingRunnable* TrackingRunnable::Runnable = NULL;
//***********************************************************

TrackingRunnable* TrackingRunnable::getInstance(cv::Mat& IN_VideoFrame, TArray<FVector2D>& IN_Vertices, int32& IN_VertexFoundCount, const int32 IN_MaxVertices)
{
	return nullptr;
};

TrackingRunnable::TrackingRunnable(cv::Mat& IN_VideoFrame, TArray<FVector2D>& IN_Vertices, int32& IN_VertexFoundCount, const int32 IN_MaxVertices)
	: VertexCount(0)
	, MaxVertices(IN_MaxVertices)
{
	VideoFrame = &IN_VideoFrame;
	Vertices = &IN_Vertices;
	VertexFoundCount = &IN_VertexFoundCount;

	Thread = FRunnableThread::Create(this, TEXT("TrackingRunnable"), 0, TPri_BelowNormal);
}

TrackingRunnable::~TrackingRunnable()
{
	delete Thread;
	Thread = NULL;
}

//Init
bool TrackingRunnable::Init()
{
	//Init the Data 

	return true;
}

uint32 TrackingRunnable::Run()
{
	// Initial wait before starting
	FPlatformProcess::Sleep(0.03);

	Completed = false;
	// While not told to stop this thread 

	while (StopTaskCounter.GetValue() == 0)
	{
		// Do main work here
		// FindFaces();
		runTracking();

		// Yield to other threads
		FPlatformProcess::Sleep(0.01);
	}

	// Run TrackingRunnable::Shutdown() from the timer in Game Thread that is watching
	// to see when TrackingRunnable::IsThreadFinished()

	return 0;
}

void TrackingRunnable::Stop()
{
	StopTaskCounter.Increment();
}

TrackingRunnable* TrackingRunnable::Instance(cv::Mat& IN_VideoFrame, TArray<FVector2D>& IN_Vertices, int32& IN_VertexFoundCount, const int32 IN_MaxVertices)
{
	//Create new instance of thread if it does not exist and the platform supports multi threading!
	if (!Runnable && FPlatformProcess::SupportsMultithreading()) {
		Runnable = getInstance(IN_VideoFrame, IN_Vertices, IN_VertexFoundCount, IN_MaxVertices);
	}
	return Runnable;
}

void TrackingRunnable::runTracking()
{
	VertexCount = 0;
	// Convert color image to grayscale
	cv::Mat gray;
	cv::cvtColor((*VideoFrame), gray, cv::COLOR_BGR2GRAY);

	// Reduce noise with a kernel 3x3
	cv::Mat detected_edges;
	cv::blur(gray, detected_edges, cv::Size(3, 3));

	// Run Canny edge detection algorithm on blurred gray image
	cv::Canny(detected_edges, detected_edges, LOW_THRESHOLD, LOW_THRESHOLD*RATIO, KERNEL_SIZE);

	// Find edges
	cv::Mat nonZeroCoordinates;
	cv::findNonZero(detected_edges, nonZeroCoordinates);

	// Store vertices (edge coordinates)
	VertexCount = min((int)nonZeroCoordinates.total(), MaxVertices);
	for (int i = 0; i < VertexCount; i++) {
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		//prevent thread from using too many resources
		FPlatformProcess::Sleep(0.01);
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		cv::Point vertex = nonZeroCoordinates.at<cv::Point>(i);
		(*Vertices)[i].X = vertex.y;
		(*Vertices)[i].Y = vertex.x;
		VertexCount++;
	}

	Completed = true;
}

void TrackingRunnable::EnsureCompletion()
{
	Stop();
	Thread->WaitForCompletion();
}

void TrackingRunnable::Shutdown()
{
	if (Runnable) {
		Runnable->EnsureCompletion();
		delete Runnable;
		Runnable = NULL;
	}
}

bool TrackingRunnable::IsThreadFinished()
{
	return Completed;
}

