// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "CV22.generated.h"

UCLASS()
class CV22HOISTSIM_API ACV22 : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACV22();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void UpdateHCPLocation();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:

	UFUNCTION(BlueprintCallable)
	void TestMeDaddy();

	UPROPERTY(EditDefaultsOnly)
	class UStaticMeshComponent* Body = nullptr;

	UPROPERTY(EditDefaultsOnly)
	class UStaticMeshComponent* RotorLeft = nullptr;

	UPROPERTY(EditDefaultsOnly)
	class UStaticMeshComponent* RotorRight = nullptr;

	UPROPERTY(EditDefaultsOnly)
	class UStaticMeshComponent* Boomhead = nullptr;

	UPROPERTY(EditDefaultsOnly)
	class UHoistControlPanel* HoistControlPanel = nullptr;

	UPROPERTY(EditDefaultsOnly)
	class UHoistComponent* Hoist = nullptr;
};
