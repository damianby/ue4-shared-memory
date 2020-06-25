// Fill out your copyright notice in the Description page of Project Settings.

#include "SharedMemory.h"


#define MUTEX_LOCK_TIMEOUT_MS 100


DEFINE_LOG_CATEGORY_STATIC(MemoryLog, Log, All);


USharedMemory::USharedMemory()
	: SharedMemoryHandle(nullptr),
	SharedMemoryData(nullptr),
	SharedMemorySize(0),
	SharedMemoryMutex(nullptr)
{
}

USharedMemory::~USharedMemory()
{
#ifdef WIN32
	if (SharedMemoryMutex != nullptr)
	{
		ReleaseMutex(SharedMemoryMutex);
		CloseHandle(SharedMemoryMutex);
		SharedMemoryMutex = nullptr;
	}

	if (SharedMemoryData != nullptr)
	{
		UnmapViewOfFile(SharedMemoryData);
		SharedMemoryData = nullptr;
	}

	if (SharedMemoryHandle != nullptr)
	{
		CloseHandle(SharedMemoryHandle);
		SharedMemoryHandle = nullptr;
	}
#endif
}


bool USharedMemory::CreateSharedMemory(const FString& sharedMemoryName, int32 size)
{
#ifdef WIN32
	/*  Create a named mutex for inter-process protection of data */
	FString mutexName = sharedMemoryName + "MUTEX";



	SharedMemoryMutex = CreateMutexW(NULL, false, *mutexName);



	if (SharedMemoryMutex == nullptr)
	{
		return false;
	}

	SharedMemorySize = size;

	SharedMemoryHandle = CreateFileMappingW((HANDLE)INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		0,
		SharedMemorySize,
		*sharedMemoryName);

	int memExists = GetLastError();

	if (memExists == ERROR_ALREADY_EXISTS || SharedMemoryHandle == nullptr)
	{
		UE_LOG(MemoryLog, Log, TEXT("Already exists or nullptr create"));
		CloseSharedMemory();
		return false;
	}

	SharedMemoryData = (unsigned char *)MapViewOfFile(SharedMemoryHandle,
		FILE_MAP_ALL_ACCESS,
		0, 0,
		SharedMemorySize);
	if (!SharedMemoryData)
	{

		UE_LOG(MemoryLog, Log, TEXT("!sharedmeomorydata"));
		CloseSharedMemory();
		return false;
	}

	LockMutex();
	std::memset(SharedMemoryData, 0, SharedMemorySize);
	UnlockMutex();

	return true;
#else
	return false;
#endif
}

bool USharedMemory::OpenSharedMemory(const FString& sharedMemoryName, int32 size)
{
#ifdef WIN32
	/*  Create a named mutex for inter-process protection of data */
	FString mutexName = sharedMemoryName + "MUTEX";


	SharedMemoryMutex = CreateMutexW(NULL, false, *mutexName);


	if (SharedMemoryMutex == nullptr)
	{
		UE_LOG(MemoryLog, Log, TEXT("mutex error"));
		return false;
	}

	SharedMemorySize = size;

	SharedMemoryHandle = OpenFileMappingW(
		FILE_MAP_ALL_ACCESS,   // read/write access
		0,                 // do not inherit the name
		*sharedMemoryName);               // name of mapping object

	int memExists = GetLastError();

	if (SharedMemoryHandle == nullptr)
	{
		UE_LOG(MemoryLog, Log, TEXT("nullptr error:  %d"), memExists);
		CloseSharedMemory();
		return false;
	}

	SharedMemoryData = (unsigned char *)MapViewOfFile(SharedMemoryHandle,
		FILE_MAP_ALL_ACCESS,
		0, 0,
		SharedMemorySize);

	if (!SharedMemoryData)
	{
		UE_LOG(MemoryLog, Log, TEXT("memory data"));
		CloseSharedMemory();
		return false;
	}

	return true;
#else
	return false;
#endif
}


void USharedMemory::WriteTransform(const FTransform& Transform)
{

	if (SharedMemoryMutex && SharedMemoryData && LockMutex())
	{
		std::memcpy(SharedMemoryData, &Transform, sizeof(FTransform));

		UnlockMutex();
	}
}

bool USharedMemory::ReadTransform(FTransform& Transform)
{
	if (SharedMemoryMutex && SharedMemoryData && LockMutex())
	{
		std::memcpy(&Transform, SharedMemoryData, sizeof(FTransform));


		UE_LOG(MemoryLog, Log, TEXT("test"));


		UnlockMutex();
		return true;
	}


	return false;
}

void USharedMemory::WriteTexture(UTextureRenderTarget2D* TextureRenderTarget)
{
#ifdef WIN32

	TArray<FColor> RawData;

	FTextureRenderTarget2DResource* RenderTarget = (FTextureRenderTarget2DResource*)TextureRenderTarget->Resource;
	EPixelFormat Format = TextureRenderTarget->GetFormat();

	int32 Width = TextureRenderTarget->SizeX;
	int32 Height = TextureRenderTarget->SizeY;

	int32 ImageBytes = Width * Height * 4;  // CalculateImageBytes(Width, Height, 0, Format);


	bool bReadSuccess = false;
	switch (Format)
	{
	case PF_FloatRGBA:
	{
		TArray<FFloat16Color> FloatColors;
		bReadSuccess = RenderTarget->ReadFloat16Pixels(FloatColors);
		FMemory::Memcpy(RawData.GetData(), FloatColors.GetData(), ImageBytes * 4);
	}
	break;
	case PF_B8G8R8A8:
		bReadSuccess = RenderTarget->ReadPixels(RawData);
	}


	int32 dataSize = RawData.Num()*RawData.GetTypeSize();

	if (SharedMemoryMutex && SharedMemoryData && LockMutex())
	{

		std::memcpy(SharedMemoryData, RawData.GetData(), dataSize);

		UnlockMutex();
	}
#endif
}

bool USharedMemory::ReadTexture(UPARAM(ref) UTexture2D* TextureIn, int32 Width, int32 Height)
{
	if (SharedMemoryMutex && SharedMemoryData && LockMutex())
	{


		FTexture2DMipMap& Mip = TextureIn->PlatformData->Mips[0];
		uint8* MipData = static_cast<uint8*>(Mip.BulkData.Lock(LOCK_READ_WRITE));


		FMemory::Memcpy(MipData, SharedMemoryData, Width * Height * 4);


		Mip.BulkData.Unlock();
#undef UpdateResource
		TextureIn->UpdateResource();

		UnlockMutex();

		return true;
	}
	return false;
}




void USharedMemory::Write(const FString& Data)
{
#ifdef WIN32

	if (Data.Len() * sizeof(TCHAR) > SharedMemorySize)
		return;

	if (SharedMemoryMutex && SharedMemoryData && LockMutex())
	{
		std::memcpy(SharedMemoryData, *Data, Data.Len() * sizeof(TCHAR));

		UnlockMutex();
	}
#endif
}




bool USharedMemory::Read(FString &Data)
{

	if (SharedMemoryMutex && SharedMemoryData && LockMutex())
	{
		Data = FString::FromBlob(SharedMemoryData, SharedMemorySize);


		UE_LOG(MemoryLog, Log, TEXT("test"));


		UnlockMutex();
		return true;
	}


	return false;
}







void USharedMemory::CloseSharedMemory()
{
#ifdef WIN32
	if (SharedMemoryMutex != nullptr)
	{
		ReleaseMutex(SharedMemoryMutex);
		CloseHandle(SharedMemoryMutex);
		SharedMemoryMutex = nullptr;
	}

	if (SharedMemoryData != nullptr)
	{
		UnmapViewOfFile(SharedMemoryData);
		SharedMemoryData = nullptr;
	}

	if (SharedMemoryHandle != nullptr)
	{
		CloseHandle(SharedMemoryHandle);
		SharedMemoryHandle = nullptr;
	}
#endif
}

bool USharedMemory::LockMutex()
{
#ifdef WIN32
	if (SharedMemoryMutex)
	{
		DWORD32 result = WaitForSingleObject(SharedMemoryMutex, MUTEX_LOCK_TIMEOUT_MS);
		if (result == WAIT_TIMEOUT)
		{
			UE_LOG(MemoryLog, Log, TEXT("MMI - VideoCapture Lock mutex timeout"));
			return false;
		}
	}

	return true;
#endif
}

void USharedMemory::UnlockMutex()
{
#ifdef WIN32
	if (SharedMemoryMutex)
	{
		ReleaseMutex(SharedMemoryMutex);
	}
#endif
}
