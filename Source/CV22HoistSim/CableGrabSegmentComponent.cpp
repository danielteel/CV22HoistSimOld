// Fill out your copyright notice in the Description page of Project Settings.


#include "CableGrabSegmentComponent.h"
#include "CableGrabComponent.h"

UCableGrabSegmentComponent::UCableGrabSegmentComponent() {
	//InitCapsuleSize(15.0f, 20.0f);
	SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	SetGenerateOverlapEvents(true);

	SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
	SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InitCapsuleSize(1.0f, 1.0f);
	//UpdateCollisionProfile();
}

void UCableGrabSegmentComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool UCableGrabSegmentComponent::GrabStart_Implementation(UPrimitiveComponent * hand) {
	UCableGrabComponent* owner = Cast<UCableGrabComponent>(GetAttachParent());
	if (owner) {
		owner->Grab(hand);
	}
	return true;
}

bool UCableGrabSegmentComponent::GrabEnd_Implementation(UPrimitiveComponent * hand) {
	UCableGrabComponent* owner = Cast<UCableGrabComponent>(GetAttachParent());
	if (owner) {
		owner->Release();
	}
	return true;
}

bool UCableGrabSegmentComponent::GrabEvent_Implementation(UPrimitiveComponent * hand, bool buttonPressed, float xAxis, float yAxis) {
	return true;
}
