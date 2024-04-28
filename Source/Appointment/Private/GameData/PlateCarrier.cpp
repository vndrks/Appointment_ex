// Fill out your copyright notice in the Description page of Project Settings.


#include "GameData/PlateCarrier.h"
#include "GameFramework/Character.h"
#include "../AppointmentPlayerController.h"

APlateCarrier::APlateCarrier()
{
	bIsEquipable = true;
}

void APlateCarrier::Use(AAppointmentPlayerController* PlayerController, bool IsInShop)
{
	if (PlayerController)
	{
		ACharacter* Character = PlayerController->GetCharacter();
		AttachToComponent(Character->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("Weapon"));
	}
}
