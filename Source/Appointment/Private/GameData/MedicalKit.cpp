// Fill out your copyright notice in the Description page of Project Settings.


#include "GameData/MedicalKit.h"
#include "../../AppointmentPlayerController.h"

AMedicalKit::AMedicalKit()
{
	HealthValue = 10.0f;
	ItemData.ItemClass = StaticClass();
}

void AMedicalKit::Use(AAppointmentPlayerController* PlayerController, bool IsInShop)
{
	Super::Use(PlayerController, IsInShop);
	if (PlayerController && !IsInShop)
	{
		PlayerController->AddHealth(HealthValue);
	}
	/*if (HasAuthority() && PlayerController)
	{
		PlayerController->AddHealth(HealthValue);
	}*/
}
