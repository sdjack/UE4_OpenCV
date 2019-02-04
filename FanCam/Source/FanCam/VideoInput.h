// Author: Steven Jackson

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FanCam.h"
#include "Tracking/FacialRecognition.h"
#include "Runtime/Engine/Classes/Engine/Texture2D.h"
#include "VideoInput.generated.h"
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>	
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>

UCLASS()
class FANCAM_API AVideoInput : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVideoInput();

	~AVideoInput();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// The device ID opened by the Video Stream
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Webcam)
		int32 CameraID;

	// The operation that will be applied to every frame
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Webcam)
		int32 OperationMode;

	// Blueprint Event called every time the video frame is updated
	UFUNCTION(BlueprintImplementableEvent, Category = Webcam)
		void OnNextVideoFrame();

	// Change OpenCV operation that will be applied to every frame
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Change Operations", Keywords = "Change Operation"), Category = Webcam)
		void ChangeOperation();

	// If the stream has succesfully opened yet
	UPROPERTY(BlueprintReadOnly, Category = Webcam)
		bool isStreamOpen;

	// If the webcam images should be resized every frame
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Webcam)
		bool ShouldResize;

	// The videos width and height (width, height)
	UPROPERTY(BlueprintReadWrite, Category = Webcam)
		FVector2D VideoSizeVector;

	// The targeted resize width and height (width, height)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Webcam)
		FVector2D VideoResizeVector;

	// The current video frame's corresponding texture
	UPROPERTY(BlueprintReadOnly, Category = Webcam)
		UTexture2D* VideoTexture;

	// The current color data array
	UPROPERTY(BlueprintReadOnly, Category = Webcam)
		TArray<FColor> VideoData;

	// The current vertex data array
	UPROPERTY(BlueprintReadOnly, Category = Webcam)
		TArray<FVector2D> Vertices;

	// The rate at which the color data array and video texture is updated (in frames per second)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Webcam)
		float RefreshRate;

	// The refresh timer
	UPROPERTY(BlueprintReadWrite, Category = Webcam)
		float RefreshTimer;

	// OpenCV prototypes
	void UpdateFrame();
	void DoProcessing();
	void UpdateTexture();
private:
	// Stores vertices computed on thread
	int MaxVertices;

protected:

	// implementation of your vision algorithm
	FacialRecognition * _algorithm;

	// The number of actual Vertices
	int32 VertexFoundCount;

	// OpenCV fields
	cv::Mat VideoFrame;
	cv::VideoCapture VideoStream;
	cv::Size VideoSize;

	// Use this function to update the texture rects you want to change:
	// NOTE: There is a method called UpdateTextureRegions in UTexture2D but it is compiled WITH_EDITOR and is not marked as ENGINE_API so it cannot be linked
	// from plugins.
	// FROM: https://wiki.unrealengine.com/Dynamic_Textures
	void UpdateTextureRegions(UTexture2D* texture, FUpdateTextureRegion2D* regions, uint32 srcpitch, uint8* srcData, bool freedata);

	// Pointer to update texture region 2D struct
	FUpdateTextureRegion2D* VideoUpdateTextureRegion;

};
