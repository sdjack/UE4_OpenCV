// Author: Steven Jackson

#pragma once

#include "FacialRecognition.h"
#include "TrackingRunnable.h"
#include "Engine.h"
#include "Runtime/Core/Public/HAL/Runnable.h"
#include "GameFramework/Actor.h"
#include <opencv2/video/tracking.hpp>
#include <opencv2/core.hpp>
#include <cstdint>

bool FacialRecognition::IncludeEyes = false;

cv::String faceCascadeFile = cv::String("cascades/haarcascades/haarcascade_frontalface_alt.xml");
cv::String eyeCascadeFile = cv::String("cascades/haarcascades/haarcascade_eye_tree_eyeglasses.xml");

FacialRecognition* FacialRecognition::getInstance(cv::Mat& IN_VideoFrame, TArray<FVector2D>& IN_Vertices, int32& IN_VertexFoundCount, const int32 IN_MaxVertices)
{
	return new FacialRecognition();
};

FacialRecognition::FacialRecognition(cv::Mat& IN_VideoFrame, TArray<FVector2D>& IN_Vertices, int32& IN_VertexFoundCount, const int32 IN_MaxVertices)
{
	Super::FacialRecognition(IN_VideoFrame, IN_Vertices, IN_VertexFoundCount, IN_MaxVertices);

	FaceCascade.load(faceCascadeFile);
	EyesCascade.load(eyeCascadeFile);
}

FacialRecognition* FacialRecognition::NewWorker(cv::Mat& IN_VideoFrame, TArray<FVector2D>& IN_Vertices, int32& IN_VertexFoundCount, const int32 IN_MaxVertices)
{
	//Create new instance of thread if it does not exist and the platform supports multi threading!
	if (!Runnable && FPlatformProcess::SupportsMultithreading()) {
		Runnable = new FacialRecognition(IN_VideoFrame, IN_Vertices, IN_VertexFoundCount, IN_MaxVertices);
	}
	return Runnable;
}

void FacialRecognition::FindEyes(cv::Mat IN_FaceFrame, const int xCoord, const int yCoord)
{
	//-- Detect eyes
	std::vector<cv::Rect> eyes;
	EyesCascade.detectMultiScale(IN_FaceFrame, eyes);

	// Store vertices (eye coordinates)
	VertexCount = min((int)eyes.size(), MaxVertices);
	for (int i = 0; i < VertexCount; i++) {
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		//prevent thread from using too many resources
		FPlatformProcess::Sleep(0.01);
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		cv::Point vertex(xCoord + eyes[i].x + eyes[i].width / 2, yCoord + eyes[i].y + eyes[i].height / 2);
		InnerVertices[i].X = vertex.y;
		InnerVertices[i].Y = vertex.x;
		InnerVertexCount++;
	}
}

void FacialRecognition::runTracking()
{
	VertexCount = 0;
	// Convert color image to grayscale
	cv::Mat gray;
	cv::cvtColor((*VideoFrame), gray, cv::COLOR_BGR2GRAY);
	cv::equalizeHist(gray, gray);

	//-- Detect faces
	std::vector<cv::Rect> faces;
	FaceCascade.detectMultiScale(gray, faces);

	// Store vertices (facial coordinates)
	VertexCount = min((int)faces.size(), MaxVertices);
	for (int i = 0; i < VertexCount; i++) {
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		//prevent thread from using too many resources
		FPlatformProcess::Sleep(0.01);
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		cv::Point vertex(faces[i].x + faces[i].width / 2, faces[i].y + faces[i].height / 2);
		(*Vertices)[i].X = vertex.y;
		(*Vertices)[i].Y = vertex.x;
		VertexCount++;

		if (IncludeEyes) {
			cv::Mat faceFrame = gray(faces[i]);
			FindEyes(faceFrame, faces[i].x, faces[i].y);
		}
	}

	Completed = true;
}

