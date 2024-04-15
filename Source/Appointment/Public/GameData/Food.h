// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameData/ApptItem.h"
#include "Food.generated.h"

/**
 * 
 */
 class AppointmentPlayerController;

UCLASS()
class APPOINTMENT_API AFood : public AApptItem
{
	GENERATED_BODY()
	
public:
	AFood();

protected:
	UPROPERTY(EditAnywhere)
	float RemoveFoodValue;

public:
	virtual void Use(AAppointmentPlayerController* PlayerController, bool IsInShop = false) override;
};
