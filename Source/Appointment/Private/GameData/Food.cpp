// Fill out your copyright notice in the Description page of Project Settings.


#include "GameData/Food.h"
#include "../../AppointmentPlayerController.h"

AFood::AFood()
{
	RemoveFoodValue = 30.0f;
	ItemData.ItemClass = StaticClass();
}

void AFood::Use(AAppointmentPlayerController* PlayerController, bool IsInShop)
{
	Super::Use(PlayerController, IsInShop);
	if (PlayerController && !IsInShop)
	{
		PlayerController->AddHealth(RemoveFoodValue);
	}
	//if (PlayerController)
	//{
	//	PlayerController->RemoveHunger(RemoveFoodValue);
	//}
}
