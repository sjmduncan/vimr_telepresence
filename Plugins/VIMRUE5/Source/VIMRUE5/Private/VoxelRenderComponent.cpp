#include "VoxelRenderComponent.h"
#include "VoxelRenderSubComponent.h"
#include "Engine.h"


UVoxelRenderComponent::UVoxelRenderComponent(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	FString rName;
	GetName(rName);
	for(int i = 0; i < MAX_RENDERER_VOXELS / SUB_VOXEL_COUNT; i++)
	{
		UVoxelRenderSubComponent* VRSC = ObjectInitializer.CreateDefaultSubobject<UVoxelRenderSubComponent>(this, FName(*(FString::Printf(TEXT("%sVoxelSubRender%d"), *rName, i))));
		VRSC->SetupAttachment(GetAttachmentRoot());
		VoxelRenderers.Add(VRSC);
	}
}


void UVoxelRenderComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if(VoxelSource != nullptr)
	{
		int VoxelCount;
		uint8 Voxelmm;
		float particleSize;
		uint8* CoarsePositionData = nullptr;
		uint8* PositionData = nullptr;
		uint8* ColourData = nullptr;
		//double startRead = FPlatformTime::Seconds();
		VoxelSource->GetFramePointers(VoxelCount, CoarsePositionData, PositionData, ColourData, Voxelmm, particleSize, RendererIdx);
		SetScale(((float)Voxelmm) / 10.0);// convert mm to cm
		if(particleSize >= 0) SetParticleSize((float)particleSize / 10.0); // convert mm to cm
		else SetParticleSize((float)Voxelmm / 10.0); // convert mm to cm

	  // Throw away any data beyond the number of pregenerated voxels
		if(VoxelCount > MAX_RENDERER_VOXELS)
		{
			VoxelCount = MAX_RENDERER_VOXELS;
		}

		int offset = 0;
		int RenderedVoxels = 0;
		bool bZero = false;
		for(auto& VRSC : VoxelRenderers)
		{
			if (bZero)
			{
				VRSC->ZeroData();
			}
			else {
				if (RenderedVoxels + SUB_VOXEL_COUNT > VoxelCount) {
					bZero = true; // Zero out further sub-renderers
					// Zero out the remainder of this one
					int SRVoxels = VoxelCount - RenderedVoxels;
					int EndOffset = offset + SRVoxels * VOXEL_TEXTURE_BPP;
					memset(CoarsePositionData + EndOffset, 0, (SUB_VOXEL_COUNT - SRVoxels) * VOXEL_TEXTURE_BPP);
					memset(PositionData + EndOffset, 0, (SUB_VOXEL_COUNT - SRVoxels) * VOXEL_TEXTURE_BPP);
				}
				// Set Data takes copies of required slices for render thread
				VRSC->SetData(CoarsePositionData + offset, PositionData + offset, ColourData + offset);
				offset += SUB_VOXEL_COUNT * VOXEL_TEXTURE_BPP;
				RenderedVoxels += SUB_VOXEL_COUNT;
			}
		}
	}
}

void UVoxelRenderComponent::InitTextures()
{
	for(auto& VRSC : VoxelRenderers)
	{
		VRSC->InitTextures();
	}
}

void UVoxelRenderComponent::SetScale(float Scale)
{
	for(auto& VRSC : VoxelRenderers)
	{
		VRSC->SetScale(Scale);
	}
}

void UVoxelRenderComponent::SetParticleSize(float ParticleSize)
{
	for(auto& VRSC : VoxelRenderers)
	{
		VRSC->SetParticleSize(ParticleSize);
	}
}

void UVoxelRenderComponent::SetLocation(FVector Location)
{
	for (auto& VRSC : VoxelRenderers)
	{
		VRSC->SetLocation(Location);
	}
}


void UVoxelRenderComponent::SetRotation(FVector Rotation)
{
	for (auto& VRSC : VoxelRenderers)
	{
		VRSC->SetRotation(Rotation);
	}
}

void UVoxelRenderComponent::SetFadeEnabled(bool enabled)
{
	for (auto& VRSC : VoxelRenderers)
	{
		VRSC->SetFadeEnabled(enabled);
	}
}

bool UVoxelRenderComponent::GetFadeEnabled()
{
	return VoxelRenderers[0]->GetFadeEnabled();
}

void UVoxelRenderComponent::SetFadeSpeed(float speed)
{
	for (auto& VRSC : VoxelRenderers)
	{
		VRSC->SetFadeSpeed(speed);
	}
}

float UVoxelRenderComponent::GetFadeSpeed()
{
	return VoxelRenderers[0]->GetFadeSpeed();
}

void UVoxelRenderComponent::SetFadeCosAng(float cosAng)
{
	for (auto& VRSC : VoxelRenderers)
	{
		VRSC->SetFadeCosAng(cosAng);
	}
}

float UVoxelRenderComponent::GetFadeCosAng()
{
	return VoxelRenderers[0]->GetFadeCosAng();
}
