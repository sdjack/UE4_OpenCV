// Author: Steven Jackson

#pragma once

#include "Engine.h"
#include "Runtime/Core/Public/HAL/Runnable.h"
#include "GameFramework/Actor.h"
#include <opencv2/core.hpp>
#include <opencv2/objdetect.hpp>

class TrackingRunnable : public FRunnable
{

public:

	TrackingRunnable(cv::Mat& IN_VideoFrame, TArray<FVector2D>& IN_Vertices, int32& IN_VertexFoundCount, const int32 IN_MaxVertices);
	virtual ~TrackingRunnable();

	// Begin FRunnable interface.
	virtual bool Init();
	virtual uint32 Run();
	virtual void Stop();
	// End FRunnable interface

	// Makes sure this thread has stopped properly 
	void EnsureCompletion();

	TrackingRunnable* getInstance(cv::Mat& IN_VideoFrame, TArray<FVector2D>& IN_Vertices, int32& IN_VertexFoundCount, const int32 IN_MaxVertices);
	/*
		Start the thread and the worker from static (easy access)!
		This code ensures only one thread will be able to run at a time.
		This function returns a handle to the newly started instance.
		*/
	static TrackingRunnable* Instance(cv::Mat& IN_VideoFrame, TArray<FVector2D>& IN_Vertices, int32& IN_VertexFoundCount, const int32 IN_MaxVertices);

	// Shuts down the thread. Static so it can easily be called from outside the thread context
	static void Shutdown();
	static bool IsThreadFinished();

protected:
	// The actual tracker processing
	virtual void runTracking(void) = 0;

	// The total allowed number of found Vertices
	int32 MaxVertices = 0;

	// The number of found Vertices
	int32 VertexCount = 0;

	// The video frame ptr
	cv::Mat* VideoFrame;

	// The vertices array ptr
	TArray<FVector2D>* Vertices;

	// The number of actual Vertices
	int32* VertexFoundCount;


private:
	// Singleton instance, can access the thread any time via static accessor, if it is active
	static TrackingRunnable* Runnable;

	// Flag to indicate a completed process
	static bool Completed;

	// Thread to run the worker FRunnable on
	FRunnableThread* Thread;

	// Stop this thread? Uses Thread Safe Counter
	FThreadSafeCounter StopTaskCounter;

	// Arbitrary edge-detection params
	const int LOW_THRESHOLD = 50;
	const int RATIO = 3;
	const int KERNEL_SIZE = 3;

	// Helper
	static int min(int a, int b) { return a < b ? a : b; }

};


