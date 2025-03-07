#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "VoxelRenderSubComponent.generated.h"


#define SUB_VOXEL_COUNT 16384
#define SUB_VOXEL_COUNT_SQR 128
#define VOXEL_TEXTURE_BPP 4

UCLASS()
class VIMRUE5_API UVoxelRenderSubComponent : public UStaticMeshComponent
{
	GENERATED_BODY()

public:
	UVoxelRenderSubComponent();

	void SetData(uint8* CoarsePositionData, uint8* PositionData, uint8* ColourData);

	void ZeroData();

	void InitTextures();

	void BeginPlay() override;

	void OnComponentDestroyed(bool bDestroyingHierarchy) override;

	void SetScale(float Scale);

	void SetParticleSize(float particleSize);

	void SetFadeSpeed(float speed);
	float GetFadeSpeed();

	void SetFadeEnabled(bool enabled);
	bool GetFadeEnabled();

	void SetFadeCosAng(float cosAng);
	float GetFadeCosAng();

	void SetLocation(FVector Location);

	void SetRotation(FVector Rotation);

private:
	UPROPERTY()
	UMaterialInterface* StaticMaterial;

	UPROPERTY()
	UMaterialInstanceDynamic* Material;

	UPROPERTY()
	UTexture2D* CoarsePositionTexture;

	UPROPERTY()
	UTexture2D* PositionTexture;

	UPROPERTY()
	UTexture2D* ColourTexture;

	FUpdateTextureRegion2D *UpdateTextureRegion;
	uint8* EmptyData;

	bool bQueueScale = false;
  bool bQueueParticleSize= false;
	bool bQueueLocation = false;
	bool bQueueRotation = false;
	bool bFadeEnabled = true;
	float fFadeSpeed = 10;
	float fFadeCosAngle = 0;
	float Scale = 1.0f;
	float ParticleSize = 1.0f;
	FVector Location{};
	FVector Rotation{};
};
