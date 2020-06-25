// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "WindowsSharedMemoryBPLibrary.h"
#include "WindowsSharedMemory.h"

UWindowsSharedMemoryBPLibrary::UWindowsSharedMemoryBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}

TArray<FString> UWindowsSharedMemoryBPLibrary::SharedMemoryList;



bool UWindowsSharedMemoryBPLibrary::CreateSharedMemory(USharedMemory*& SharedMemory, FString Name, int32 Size)
{
	if (SharedMemoryList.Contains(Name)) {
		UE_LOG(MemoryLog, Log, TEXT("Shared Memory with given name exists!"));
		return false;
	}

	SharedMemoryList.Add(Name);

	SharedMemory = NewObject<USharedMemory>();
	
	if (!SharedMemory->CreateSharedMemory(Name, Size)) {
		SharedMemory->BeginDestroy();
		return false;
	}

	
	return true;
}

bool UWindowsSharedMemoryBPLibrary::OpenSharedMemory(USharedMemory*& SharedMemory, FString Name, int32 Size)
{
	if (SharedMemoryList.Contains(Name)) {
		UE_LOG(MemoryLog, Log, TEXT("Shared Memory with given name exists!"));
		return false;
	}

	SharedMemoryList.Add(Name);

	SharedMemory = NewObject<USharedMemory>();

	if (SharedMemory->OpenSharedMemory(Name, Size)) {
		SharedMemory->BeginDestroy();
		return false;
	}


	return true;
}


UTexture2D* UWindowsSharedMemoryBPLibrary::CreateNewTexture2D(const int32& Width, const int32& Height)
{

	UTexture2D* Texture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);

	//Valid?
	if (!Texture) {
		UE_LOG(MemoryLog, Log, TEXT("TEX NULL data"));
		return NULL;
	}
	//~~~~~~~~~~~~~~


	TArray<FColor> Data;

	Data.Init(FColor(0, 0, 0, 0), Width * Height * 4);

	//Copy!
	void* TextureData = Texture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(TextureData, Data.GetData(), Data.Num());
	Texture->PlatformData->Mips[0].BulkData.Unlock();

	//Update!

#undef UpdateResource
	Texture->UpdateResource();






	return Texture;
}
