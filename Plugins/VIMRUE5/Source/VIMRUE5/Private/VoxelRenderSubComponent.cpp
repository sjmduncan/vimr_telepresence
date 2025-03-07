#include "VoxelRenderSubComponent.h"
#include "VIMRUE5.h"
#include "Engine.h"

// Fix conflict between Windows.h macros and Unreal function name
#undef UpdateResource



UVoxelRenderSubComponent::UVoxelRenderSubComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	FString VoxelAsset = FString::Printf(TEXT("StaticMesh'/VIMRUE5/UnitCubesOffset.UnitCubesOffset'"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshFinder(*VoxelAsset);
	if(MeshFinder.Object != nullptr)
	{
		SetStaticMesh(MeshFinder.Object);
	}
	else
	{
		UE_LOG(VIMRLog, Error, TEXT("Couldn't load asset: %s"), *VoxelAsset);
	}

	static ConstructorHelpers::FObjectFinder<UMaterial> MaterialFinder(TEXT("Material'/VIMRUE5/VertexMoveMaterial.VertexMoveMaterial'"));
	if(MaterialFinder.Object != nullptr)
	{
		StaticMaterial = MaterialFinder.Object;
	}
	else
	{
		UE_LOG(VIMRLog, Error, TEXT("Couldn't load /VIMRUE5/VertexMoveMaterial"));
	}

	UpdateTextureRegion = new FUpdateTextureRegion2D(0, 0, 0, 0, SUB_VOXEL_COUNT_SQR, SUB_VOXEL_COUNT_SQR);

	EmptyData = new uint8[SUB_VOXEL_COUNT * VOXEL_TEXTURE_BPP]();
}

void UVoxelRenderSubComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	if(UpdateTextureRegion != nullptr)
	{
		delete UpdateTextureRegion;
		UpdateTextureRegion = nullptr;
	}

	delete[] EmptyData;

	Super::OnComponentDestroyed(bDestroyingHierarchy);
}

void UVoxelRenderSubComponent::BeginPlay()
{
	Super::BeginPlay();
	InitTextures();
}

void UVoxelRenderSubComponent::SetData(uint8* CoarsePositionData, uint8* PositionData, uint8* ColourData)
{
	if(CoarsePositionTexture && PositionTexture && ColourTexture)
	{
		// Take heap allocated copy of input data to hand to render thread, which will be responsible for cleanup in lambda
		// TODO: Restructure so that caller moves memory responsibility to here, avoiding extra allocation, remove delete in callback once done
		size_t DataSize = SUB_VOXEL_COUNT * VOXEL_TEXTURE_BPP;
		uint8* PositionDataRT = new uint8[DataSize];
		memcpy(PositionDataRT, PositionData, DataSize);
		uint8* ColourDataRT = new uint8[DataSize];
		memcpy(ColourDataRT, ColourData, DataSize);
		uint8* CoarseDataRT = new uint8[DataSize];
		memcpy(CoarseDataRT, CoarsePositionData, DataSize);

		// Region is safely reused across calls, it isn't modified and doesn't need to be deleted
		CoarsePositionTexture->UpdateTextureRegions(0, 1, UpdateTextureRegion, SUB_VOXEL_COUNT_SQR * VOXEL_TEXTURE_BPP, VOXEL_TEXTURE_BPP, CoarseDataRT, [](uint8 *Data, const FUpdateTextureRegion2D *Region) {
			delete[] Data;
		});
		PositionTexture->UpdateTextureRegions(0, 1, UpdateTextureRegion, SUB_VOXEL_COUNT_SQR * VOXEL_TEXTURE_BPP, VOXEL_TEXTURE_BPP, PositionDataRT, [](uint8 *Data, const FUpdateTextureRegion2D *Region) {
			delete[] Data;
		});
		ColourTexture->UpdateTextureRegions(0, 1, UpdateTextureRegion, SUB_VOXEL_COUNT_SQR * VOXEL_TEXTURE_BPP, VOXEL_TEXTURE_BPP, ColourDataRT, [](uint8 *Data, const FUpdateTextureRegion2D *Region) {
			delete[] Data;
		});
	}
	else
	{
		UE_LOG(VIMRLog, Warning, TEXT("Tried UVoxelRenderSubComponent::SetData without Textures initalised"));
		InitTextures();
	}
}

void UVoxelRenderSubComponent::ZeroData()
{
	if (CoarsePositionTexture && PositionTexture)
	{
		CoarsePositionTexture->UpdateTextureRegions(0, 1, UpdateTextureRegion, SUB_VOXEL_COUNT_SQR * VOXEL_TEXTURE_BPP, VOXEL_TEXTURE_BPP, EmptyData, [](uint8 *Data, const FUpdateTextureRegion2D *Region) {});
		PositionTexture->UpdateTextureRegions(0, 1, UpdateTextureRegion, SUB_VOXEL_COUNT_SQR * VOXEL_TEXTURE_BPP, VOXEL_TEXTURE_BPP, EmptyData, [](uint8 *Data, const FUpdateTextureRegion2D *Region) {});
	}
}

void UVoxelRenderSubComponent::InitTextures()
{
	
	SetBoundsScale(10000.0f); // TODO: Automatically match this to voxel mat properties

	if(StaticMaterial)
	{
		CoarsePositionTexture = UTexture2D::CreateTransient(SUB_VOXEL_COUNT_SQR, SUB_VOXEL_COUNT_SQR);
		CoarsePositionTexture->UpdateResource();

		PositionTexture = UTexture2D::CreateTransient(SUB_VOXEL_COUNT_SQR, SUB_VOXEL_COUNT_SQR);
		PositionTexture->UpdateResource();

		ColourTexture = UTexture2D::CreateTransient(SUB_VOXEL_COUNT_SQR, SUB_VOXEL_COUNT_SQR);
		ColourTexture->UpdateResource();

		Material = CreateDynamicMaterialInstance(0, StaticMaterial);
		Material->SetTextureParameterValue(FName("CoarsePositionTexture"), CoarsePositionTexture);
		Material->SetTextureParameterValue(FName("PositionTexture"), PositionTexture);
		Material->SetTextureParameterValue(FName("ColourTexture"), ColourTexture);
		SetMaterial(0, Material);
		// GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, FString("Added Textures to Material"));

		// Scale was set before Material was available (e.g. in a parent constructor)
		if (bQueueScale)
		{
			Material->SetScalarParameterValue(FName("Scale"), Scale);
			bQueueScale = false;
		}
				// Scale was set before Material was available (e.g. in a parent constructor)
		if (bQueueParticleSize)
		{
      Material->SetScalarParameterValue(FName("ParticleSize"), ParticleSize);
			bQueueParticleSize = false;
		}
		if (bQueueLocation)
		{
			Material->SetVectorParameterValue(FName("Location"), Location);
			bQueueLocation = false;
		}
    if (bQueueRotation)
		{
			Material->SetVectorParameterValue(FName("Rotation"), Rotation);
			bQueueRotation = false;
		}
	}
	else
	{
		UE_LOG(VIMRLog, Warning, TEXT("Tried UVoxelRenderSubComponent::BeginPlay without Material ready"));
	}
}

void UVoxelRenderSubComponent::SetScale(float scale)
{
	if (scale != this->Scale) {
		this->Scale = scale;
		if (Material != nullptr)
		{
			Material->SetScalarParameterValue(FName("Scale"), scale);
		}
		else
		{
			bQueueScale = true;
		}
	}
}

void UVoxelRenderSubComponent::SetParticleSize(float particleSize)
{
		if (particleSize != this->ParticleSize) {
		this->ParticleSize = particleSize;
		if (Material != nullptr)
		{
			Material->SetScalarParameterValue(FName("ParticleSize"), particleSize);
		}
		else
		{
			bQueueParticleSize = true;
		}
	}
}

void UVoxelRenderSubComponent::SetFadeSpeed(float speed)
{
	 if (speed != this->fFadeSpeed) {
		this->fFadeSpeed = speed;;
		if (Material != nullptr)
		{
			Material->SetScalarParameterValue(FName("GreyOutSpeed"), speed);
		}
		else
		{
			bQueueParticleSize = true;
		}
	}
}

float UVoxelRenderSubComponent::GetFadeSpeed()
{
	return this->fFadeSpeed;
}

void UVoxelRenderSubComponent::SetFadeEnabled(bool enabled)
{
  if (enabled != this->bFadeEnabled) {
		this->bFadeEnabled = enabled;
		if (Material != nullptr)
		{
			Material->SetScalarParameterValue(FName("GreyOutEnabled"), enabled * 1);
		}
		else
		{
			bQueueParticleSize = true;
		}
	}
}

bool UVoxelRenderSubComponent::GetFadeEnabled()
{
	return this->bFadeEnabled;
}

void UVoxelRenderSubComponent::SetFadeCosAng(float cosAng)
{
	if (cosAng != this->fFadeCosAngle) {
		this->fFadeCosAngle = cosAng;
		if (Material != nullptr)
		{
			Material->SetScalarParameterValue(FName("GreyOutCosAngle"), cosAng);
		}
		else
		{
			bQueueParticleSize = true;
		}
	}
}

float UVoxelRenderSubComponent::GetFadeCosAng()
{
	return this->fFadeCosAngle;
}

void UVoxelRenderSubComponent::SetLocation(FVector location)
{
	this->Location = location;
	if (Material != nullptr)
	{
		Material->SetVectorParameterValue(FName("Location"), location);
	}
	else
	{
		bQueueLocation = true;
	}
}
void UVoxelRenderSubComponent::SetRotation(FVector rotation)
{
  this->Rotation = rotation;
  if (Material != nullptr)
  {
	Material->SetVectorParameterValue(FName("Rotation"), rotation);
  }
  else
  {
	bQueueRotation = true;
  }
}
