// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameData/ApptItem.h"
#include "Gold.generated.h"

/**
 * 
 */
UCLASS()
class APPOINTMENT_API AGold : public AApptItem
{
	GENERATED_BODY()

public:
	AGold();


public:
	virtual void Use(AAppointmentPlayerController* PlayerController) override;
	
};
