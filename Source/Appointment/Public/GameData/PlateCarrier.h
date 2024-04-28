// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameData/ApptItem.h"
#include "PlateCarrier.generated.h"

/**
 * 
 */
UCLASS()
class APPOINTMENT_API APlateCarrier : public AApptItem
{
	GENERATED_BODY()
	
public:
	APlateCarrier();
	virtual void Use(class AAppointmentPlayerController* PlayerController, bool IsInShop = false);

	
};
