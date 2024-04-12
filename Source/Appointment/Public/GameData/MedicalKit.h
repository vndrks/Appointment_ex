// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameData/ApptItem.h"
#include "MedicalKit.generated.h"


/**
 * 
 */
class AAppointmentPlayerController;

UCLASS()
class APPOINTMENT_API AMedicalKit : public AApptItem
{
	GENERATED_BODY()
	
public:
	AMedicalKit();

protected:
	UPROPERTY(EditAnyWhere)
	float HealthValue;

public:
	virtual void Use(AAppointmentPlayerController* PlayerController, bool IsInShop = false) override;
};
