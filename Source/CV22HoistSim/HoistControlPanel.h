#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "HoistControlPanel.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CV22HOISTSIM_API UHoistControlPanel : public UStaticMeshComponent {
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UHoistControlPanel();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	class UHoistComponent* GetHoistComponent();

	bool PowerIsOn = false;

	float ExtendCommand = 0.0f;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void SetPowerState(bool power);

	UFUNCTION(BlueprintCallable)
	bool GetPowerState();

	UFUNCTION(BlueprintCallable)
	void SetExtendCommand(float extend);

	UFUNCTION(BlueprintCallable)
	void Jettison();

	UPROPERTY(EditDefaultsOnly)
	class UStaticMeshComponent* JettisonMeshComponent = nullptr;

	UPROPERTY(EditDefaultsOnly)
	class UStaticMeshComponent* PowerSwitchMeshComponent = nullptr;

	UPROPERTY(EditDefaultsOnly)
	class UStaticMeshComponent* UpDownMeshComponent = nullptr;
};
