// Fill out your copyright notice in the Description page of Project Settings.


#include "CV22.h"
#include "Components/StaticMeshComponent.h"
#include "HoistControlPanel.h"
#include "HoistComponent.h"
#include "TailScanner.h"
#include "RotorComponent.h"
#include "HandControllerComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ACV22::ACV22()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Body = CreateDefaultSubobject<UStaticMeshComponent>(FName("Body"));
	SetRootComponent(Body);

	RotorLeft = CreateDefaultSubobject<UStaticMeshComponent>(FName("RotorLeft"));
	RotorLeft->SetupAttachment(Body, FName("RotorLeft"));

	RotorRight = CreateDefaultSubobject<UStaticMeshComponent>(FName("RotorRight"));
	RotorRight->SetupAttachment(Body, FName("RotorRight"));

	Boomhead = CreateDefaultSubobject<UStaticMeshComponent>(FName("Boomhead"));
	Boomhead->SetupAttachment(Body, FName("Boomhead"));

	HoistControlPanel = CreateDefaultSubobject<UHoistControlPanel>(FName("HoistControlPanel"));
	HoistControlPanel->SetupAttachment(Body, "HoistControlPanel");

	Hoist = CreateDefaultSubobject<UHoistComponent>(FName("Hoist"));
	Hoist->SetupAttachment(Boomhead, "Hoist");

	Tags.Add(FName("CanBeAt"));
}

// Called when the game starts or when spawned
void ACV22::BeginPlay()
{
	Super::BeginPlay();
}

void ACV22::TestMeDaddy() {

}

void ACV22::UpdateHCPLocation() {
	ATailScanner* TailScanner = Cast<ATailScanner>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	if (TailScanner) {
		UHandControllerComponent* RightController = TailScanner->GetRightController();
		if (RightController && RightController->GetEnabled() == false) {
			FVector NewHCPLocation = RightController->GetComponentLocation();
			HoistControlPanel->SetWorldLocation(NewHCPLocation);
			HoistControlPanel->SetWorldRotation(RightController->GetComponentRotation());
			FRotator RotOffset = FRotator(0, 90, -45);
			HoistControlPanel->AddLocalRotation(RotOffset);
			FVector Offset = FVector(-4.5f, 5.5f, -16.0f);
			HoistControlPanel->AddLocalOffset(Offset);

			return;
		}
	}
	HoistControlPanel->SetWorldLocation(Body->GetSocketLocation("HoistControlPanel"));
	HoistControlPanel->SetWorldRotation(Body->GetSocketRotation("HoistControlPanel"));
}


// Called every frame
void ACV22::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateHCPLocation();

	RotorLeft->AddLocalRotation(FRotator(0.0f, 0, 1700 * DeltaTime));
	RotorRight->AddLocalRotation(FRotator(0.0f, 0, -1700 * DeltaTime));
}

// Called to bind functionality to input
void ACV22::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

