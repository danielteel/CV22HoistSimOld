// Fill out your copyright notice in the Description page of Project Settings.


#include "HoistComponent.h"
#include "Engine/Classes/PhysicsEngine/PhysicsConstraintComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "CableComponent.h"
#include "DrawDebugHelpers.h"
#include "Physics/PhysScene_PhysX.h"
#include "Physics/PhysicsInterfaceCore.h"
#include "PhysXPublic.h"
#include "PhysicsEngine/BodyInstance.h"
#include "GameFramework/PhysicsVolume.h"
#include "Kismet/KismetMathLibrary.h"
#include "CableGrabComponent.h"

// Sets default values for this component's properties
UHoistComponent::UHoistComponent() {
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	HookMesh = CreateDefaultSubobject<UStaticMesh>(FName("HookMesh"));

	CableBase = CreateDefaultSubobject<UStaticMeshComponent>(FName("CableBase"));
	CableBase->SetupAttachment(this);
	RescueHook = CreateDefaultSubobject<URescueHook>(FName("RescueHook"));
	RescueHook->SetupAttachment(CableBase);

	CableBaseConstraint = CreateDefaultSubobject<UPhysicsConstraintComponent>(FName("CableBaseConstraint"));
	CableBaseConstraint->SetupAttachment(CableBase);

	BaseToHookCable = CreateDefaultSubobject<UCableComponent>(FName("BaseToHookCable"));
	BaseToHookCable->SetupAttachment(CableBase);
	BaseToHookCable->CableLength = 1.0f;
	BaseToHookCable->NumSegments = 70;
	BaseToHookCable->bEnableStiffness = true;
	BaseToHookCable->SolverIterations = 16;
	BaseToHookCable->SubstepTime = 0.01;
	BaseToHookCable->SetEnableGravity(true);
	BaseToHookCable->bEnableCollision = true;
	BaseToHookCable->EndLocation = FVector(0.0f);
	BaseToHookCable->SetAttachEndToComponent(RescueHook);
	BaseToHookCable->bCastFarShadow = true;
	BaseToHookCable->bCastDynamicShadow = true;
	BaseToHookCable->CableWidth = 4.0f;
	BaseToHookCable->CollisionFriction = 0.05f;

	BoomToBaseCable = CreateDefaultSubobject<UCableComponent>(FName("BoomToBaseCable"));
	BoomToBaseCable->SetupAttachment(this);
	BoomToBaseCable->CableLength = 1.0f;
	BoomToBaseCable->NumSegments = 1;
	BoomToBaseCable->SubstepTime = BaseToHookCable->SubstepTime;
	BoomToBaseCable->SetEnableGravity(true);
	BoomToBaseCable->EndLocation = FVector(0.0f);
	BoomToBaseCable->SetAttachEndToComponent(CableBase);
	BoomToBaseCable->CableWidth = BaseToHookCable->CableWidth;

	CableGrabber = CreateDefaultSubobject<UCableGrabComponent>(FName("CableGrabber"));
	CableGrabber->SetupAttachment(this);
}


// Called when the game starts
void UHoistComponent::BeginPlay() {
	Super::BeginPlay();
	
	CableGrabber->Setup(BaseToHookCable);

	CableBase->SetStaticMesh(CableBaseMesh);
	CableBase->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CableBase->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	CableBase->SetVisibility(false);


	RescueHook->SetStaticMesh(HookMesh);
	RescueHook->SetMassOverrideInKg(NAME_None, 3.0f);
	RescueHook->SetSimulatePhysics(true);
	RescueHook->SetUseCCD(true);
	RescueHook->bCastFarShadow = true;
	RescueHook->bCastDynamicShadow = true;

	FVector hookToLocation = GetComponentLocation();
	hookToLocation += -GetUpVector()*1.0f;
	FRotator hookToRotation = GetComponentRotation();
	RescueHook->SetWorldLocationAndRotation(hookToLocation,hookToRotation);

	CableBaseConstraint->SetWorldLocation(GetComponentLocation());
	CableBaseConstraint->SetLinearXLimit(ELinearConstraintMotion::LCM_Limited, 0.0f);
	CableBaseConstraint->SetLinearYLimit(ELinearConstraintMotion::LCM_Limited, 0.0f);
	CableBaseConstraint->SetLinearZLimit(ELinearConstraintMotion::LCM_Limited, 0.0f);
	CableBaseConstraint->ConstraintInstance.ProfileInstance.LinearLimit.bSoftConstraint = 0;
	CableBaseConstraint->ConstraintInstance.ProfileInstance.LinearLimit.Restitution = 0.0f;
	CableBaseConstraint->ConstraintInstance.ProfileInstance.LinearLimit.ContactDistance = 100.0f;
	CableBaseConstraint->SetConstrainedComponents(CableBase, NAME_None, RescueHook, NAME_None);
	CableBaseConstraint->Activate();
}


void UHoistComponent::Extend(float value) {
	ExtendCommand = FMath::Clamp(value, -1.0f, 1.0f);
}

float UHoistComponent::GetHoistOutLength() {
	return HoistOutLength;
}

bool UHoistComponent::IsWithinLimit() {
	if (HoistOutLength<HoistLimitDist || HoistOutLength>(HoistMaxLength - HoistLimitDist)) {
		return true;
	}
	return false;
}

void UHoistComponent::UpdateCableGrab(float deltaTime) {
	bool isGrabbed = CableGrabber->IsGrabbed();
	if (isGrabbed != LastGrabState) GrabConditionMet = false;

	LastGrabState = isGrabbed;

	FVector endLocation;
	if (isGrabbed) {
		endLocation = CableGrabber->GrabbedHand->GetComponentLocation();
	} else {
		endLocation = GetComponentLocation();
	}

	if (GrabConditionMet) {
		CableBase->SetWorldLocation(endLocation);
	} else {
		float dist = FVector::Dist(CableBase->GetComponentLocation(), endLocation);

		if (dist < GrabConditionDelta) {
			GrabConditionMet = true;
		} else {
			float moveDistance = FMath::Min(dist, GrabSpeed*deltaTime);
			FVector moveNormal = (endLocation - CableBase->GetComponentLocation()).GetSafeNormal();
			CableBase->AddWorldOffset(moveNormal*moveDistance);
		}
	}

	HoistGrabOffset = FVector::Dist(CableBase->GetComponentLocation(), GetComponentLocation());

	if (HoistGrabOffset > HoistOutLength) {
		FVector offsetNormal = (endLocation - GetComponentLocation()).GetSafeNormal();
		CableBase->SetWorldLocation((offsetNormal*HoistOutLength) + GetComponentLocation());
	}
}

void UHoistComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	FixStuckCable();

	if (Jettisoned) return;

	UpdateCableGrab(DeltaTime);


	FMath::SRandInit((int32)(GetWorld()->TimeSeconds));
	BaseToHookCable->CableForce = (FVector::ZeroVector - GetForwardVector())*CableWindForce*(FMath::SRand() - 0.2f);

	float extendDistance;//Determine hoist cable speed, if its within 14ft of begining or end, limit the speed
	if (HoistOutLength > HoistLimitDist && HoistOutLength < (HoistMaxLength - HoistLimitDist)) {
		extendDistance = ExtendCommand * (DeltaTime*HoistMaxSpeed);
	} else {
		extendDistance = ExtendCommand * (DeltaTime*HoistLimitMaxSpeed);
	}

	SetHoistLength(HoistOutLength + extendDistance);

	CableBase->AddLocalRotation(FRotator(0, 0.01*DeltaTime, 0));
}



void UHoistComponent::SetHoistLength(float hoistLength) {
	if (Jettisoned) return;
	HoistOutLength = FMath::Clamp(hoistLength, 0.0f, HoistMaxLength);

	float hoistOutMinusOffset = HoistOutLength - HoistGrabOffset;

	CableBaseConstraint->SetLinearXLimit(ELinearConstraintMotion::LCM_Limited, hoistOutMinusOffset);
	CableBaseConstraint->SetLinearYLimit(ELinearConstraintMotion::LCM_Limited, hoistOutMinusOffset);
	CableBaseConstraint->SetLinearZLimit(ELinearConstraintMotion::LCM_Limited, hoistOutMinusOffset);

	BaseToHookCable->CableLength = hoistOutMinusOffset - 40.0f;

	if (HoistGrabOffset < 1.0f) {
		BoomToBaseCable->SetVisibility(false);
	} else {
		BoomToBaseCable->SetVisibility(true);
	}

	if (hoistOutMinusOffset < 1.0f) {
		BaseToHookCable->SetVisibility(false);
	} else {
		BaseToHookCable->SetVisibility(true);
	}

}

bool UHoistComponent::IsJettisoned() {
	return Jettisoned;
}

void UHoistComponent::JettisonHoist() {
	if (Jettisoned) return;
	Jettisoned = true;

	CableBaseConstraint->BreakConstraint();
	CableBase->SetSimulatePhysics(true);
	CableBase->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	BaseToHookCable->bEnableCollision = true;
	BaseToHookCable->bAttachStart = false;
	BaseToHookCable->CableForce = FVector::ZeroVector;
	BoomToBaseCable->SetVisibility(false);
}

TArray<class UPrimitiveComponent*> UHoistComponent::GetWindAffectableComponents() {
	TArray<UPrimitiveComponent*> windAffectableComponents;
	windAffectableComponents.Add(RescueHook);
	if (RescueHook->GetAttachedDevice()) {
		windAffectableComponents.Add(RescueHook->GetAttachedDevice());
	}
	return windAffectableComponents;
}



UPrimitiveComponent* UHoistComponent::GetDeviceOnGround() {
	if (Jettisoned) return nullptr;
	UPrimitiveComponent* returnComponent = nullptr;
	bool traceResult = false;
	FHitResult hitResult;
	FCollisionQueryParams params;
	params.AddIgnoredActor(GetOwner());
	params.AddIgnoredComponent(RescueHook);

	FVector start, end;
	if (RescueHook->GetAttachedDevice()) {
		params.AddIgnoredComponent(RescueHook->GetAttachedDevice());
		returnComponent = RescueHook->GetAttachedDevice();
	} else {
		returnComponent = RescueHook;
	}
	start = returnComponent->GetComponentLocation();
	end = start - FVector::UpVector*HoistMaxLength;

	traceResult = GetWorld()->LineTraceSingleByChannel(hitResult, start, end, ECollisionChannel::ECC_Visibility);
	if (traceResult && hitResult.Distance < 30.0f) {//We hit the ground and it was less then 2 feet away
		return returnComponent;
	}
	return nullptr;
}

UPrimitiveComponent * UHoistComponent::GetAttachedDevice() {
	return RescueHook->GetAttachedDevice();
}



void UHoistComponent::FixStuckCable() {
	TArray<FVector> cableParticleLocations;
	bool noStuckParticles = true;
	BaseToHookCable->GetCableParticleLocations(cableParticleLocations);
	if (cableParticleLocations.Num() > 2) {
		float lastDistanceBetween = 0.0f;
		float distanceBetween = 0.0f;
		float particleDeltaSlop = 10.0f;//10 cm variation allowed between particles
		for (int i = 0; i < cableParticleLocations.Num() - 1; i++) {
			distanceBetween = FVector::Dist(cableParticleLocations[i], cableParticleLocations[i + 1]);
			if (i > 0) {
				if (FMath::Abs(distanceBetween - lastDistanceBetween) > particleDeltaSlop) {
					noStuckParticles = false;
					break;
				}
			}
			lastDistanceBetween = distanceBetween;
		}
		if (noStuckParticles) {
			BaseToHookCable->bEnableCollision = true;
		} else {
			BaseToHookCable->bEnableCollision = false;
		}
	}
}