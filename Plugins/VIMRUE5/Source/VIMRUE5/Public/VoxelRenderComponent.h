#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "VoxelSourceInterface.h"
#include "VoxelRenderComponent.generated.h"


#define MAX_RENDERER_VOXELS (12 * 16384)
//12*16384 = 196608 = absolute max voxels per renderer
//To further increase max voxel count change the NumVoxels variable in VIMRActor.h

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VIMRUE5_API UVoxelRenderComponent : public USceneComponent
{
	GENERATED_UCLASS_BODY()

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void InitTextures();

	UFUNCTION(BlueprintCallable, Category = "VIMRRender")
	void SetScale(float Scale);

	UFUNCTION(BlueprintCallable, Category = "VIMRRender")
	void SetParticleSize(float ParticleSize);

	UFUNCTION(BlueprintCallable, Category = "VIMRRender")
	void SetLocation(FVector Location);

	UFUNCTION(BlueprintCallable, Category = "VIMRRender")
	void SetRotation(FVector Rotation);

	UFUNCTION(BlueprintCallable, Category = "VIMRRender")
	void SetFadeEnabled(bool enabled);
  UFUNCTION(BlueprintCallable, Category = "VIMRRender")
	bool GetFadeEnabled();

	UFUNCTION(BlueprintCallable, Category = "VIMRRender")
	void SetFadeSpeed(float speed);
	UFUNCTION(BlueprintCallable, Category = "VIMRRender")
	float GetFadeSpeed();

	UFUNCTION(BlueprintCallable, Category = "VIMRRender")
	void SetFadeCosAng(float cosAng);
	UFUNCTION(BlueprintCallable, Category = "VIMRRender")
	float GetFadeCosAng();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Default)
	TScriptInterface<IVoxelSourceInterface> VoxelSource;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Default)
	int RendererIdx;

private:
	UPROPERTY()
	TArray<class UVoxelRenderSubComponent*> VoxelRenderers;
};
