// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Interface/InteractableInterface.h"
#include "../GameData/ApptData.h"

#include "ShopKeeper.generated.h"

UCLASS()
class APPOINTMENT_API AShopKeeper : public AActor, public IInteractableInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AShopKeeper();

protected:
	UPROPERTY(EditDefaultsOnly)
	class USkeletalMeshComponent* ShopKeeperMesh;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TArray<FItemData> Items;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	virtual void Interact(class AAppointmentPlayerController* PlayerController) override;
};
