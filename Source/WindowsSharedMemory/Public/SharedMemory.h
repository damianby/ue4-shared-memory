// Fill out your copyright notice in the Description page of Project Settings.

#pragma once



#include "UnrealString.h"
#include "Color.h"
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine.h"


#include "Engine/TextureRenderTarget2D.h"
#include "RenderUtils.h"
#include "Kismet/BlueprintFunctionLibrary.h"


#define WIN32_LEAN_AND_MEAN

#ifdef WIN32
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <memory>
#endif



#include "UObject/NoExportTypes.h"
#include "SharedMemory.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, ClassGroup = (Custom))
class WINDOWSSHAREDMEMORY_API USharedMemory : public UObject
{
	GENERATED_BODY()

public:
	USharedMemory();
	~USharedMemory();


	UFUNCTION(BlueprintInternalUseOnly, Category = "Shared Memory")
		bool CreateSharedMemory(const FString& sharedMemoryName, int32 size);

	UFUNCTION(BlueprintInternalUseOnly, Category = "Shared Memory")
		bool OpenSharedMemory(const FString& sharedMemoryName, int32 size);

	UFUNCTION(BlueprintCallable, Category = "Shared Memory")
		void CloseSharedMemory();



	UFUNCTION(BlueprintCallable, Category = "Shared Memory")
		void Write(const FString& Data);

	UFUNCTION(BlueprintCallable, Category = "Shared Memory")
		void WriteTransform(const FTransform& Transform);

	UFUNCTION(BlueprintCallable, Category = "Shared Memory")
		bool ReadTransform(FTransform& Transform);


	UFUNCTION(BlueprintCallable, Category = "Shared Memory")
		void WriteTexture(UTextureRenderTarget2D* TextureRenderTarget);


	// Writes texture from Shared Memory to Render Target
	UFUNCTION(BlueprintCallable, Category = "Shared Memory")
		bool ReadTexture(UPARAM(ref) UTexture2D* TextureIn, int32 Width, int32 Height);


	UFUNCTION(BlueprintCallable, Category = "Shared Memory")
		bool Read(FString &Data);





private: 

	void* SharedMemoryHandle;           //  Mapped memory handle.
	unsigned char* SharedMemoryData;	//  Pointer to memory data.
	size_t SharedMemorySize;            //  Length of the memory (memory size).
	void* SharedMemoryMutex;            // Mutex handle.


	bool LockMutex();
	void UnlockMutex();

};
