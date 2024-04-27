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

	UPROPERTY(ReplicatedUsing = OnRep_Items, EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TArray<FItemData> Items;

	UFUNCTION()
	void OnRep_Items();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	void TransfferedItem(TSubclassOf<AApptItem> ItemSubclass);

	bool CanBuyItem(int32 CurrentGold, TSubclassOf<AApptItem> ItemSubclass);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	virtual void Interact(class AAppointmentPlayerController* PlayerController) override;


	
	bool BuyItem(class AAppointmentPlayerController* PlayerController, TSubclassOf<AApptItem> ItemSubclass);
};
