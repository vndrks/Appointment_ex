// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "Public/GameData/ApptItem.h"
#include "AppointmentPlayerController.generated.h"

/** Forward declaration to improve compiling times */
class UNiagaraSystem;
class UInputMappingContext;
class UInputAction;
class AShopKeeper;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS()
class AAppointmentPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AAppointmentPlayerController();

	/** Time Threshold to know if it was a short press */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	float ShortPressThreshold;

	/** FX Class that we will spawn when clicking */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UNiagaraSystem* FXCursor;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;
	
	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* SetDestinationClickAction;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* SetDestinationTouchAction;

	UFUNCTION(BlueprintImplementableEvent)
	void OpenShop(AShopKeeper* OwningShop, const TArray<FItemData>& Items);

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateShop(const TArray<FItemData>& Items);

	void AddInventoryItem(FItemData ItemData);
	void AddHealth(float Value);
	void RemoveHunger(float Value);

	UFUNCTION(BlueprintCallable)
	int32 GetCurrentGold();

	UFUNCTION(BlueprintCallable)
	void RemoveGold(int32 AmountToRemove);

protected:
	/** True if the controlled character should navigate to the mouse cursor. */
	uint32 bMoveToMouseCursor : 1;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* SetKeyboardMoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* InteractAction;

	UPROPERTY(ReplicatedUsing = OnRep_InventoryItems, BlueprintReadWrite, Category = "Inventory")
	TArray<FItemData> InventoryItems;

	UPROPERTY(ReplicatedUsing = OnRep_Stats, BlueprintReadWrite, Category = "Inventory")
	float Health;

	UPROPERTY(ReplicatedUsing = OnRep_Stats, BlueprintReadWrite, Category = "Inventory")
	float Hunger;

	virtual void SetupInputComponent() override;
	
	// To add mapping context
	virtual void BeginPlay();

	/** Input handlers for SetDestination action. */
	void OnInputStarted();
	void OnSetDestinationTriggered();
	void OnSetDestinationReleased();
	void OnTouchTriggered();
	void OnTouchReleased();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Interact(FVector Start, FVector End, AActor* HitActor);

	/** Interaction with field's items */
	// void Interact();
	void Interact(FVector Start, FVector End, AActor* HitActor);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_UseItem(TSubclassOf<AApptItem> ItemSubclass, AShopKeeper* ShopKeeper, bool IsShopItem = false);

	void UseRemoveItem(TSubclassOf<AApptItem> ItemSubclass, bool UseItem /* Item : true, Gold : false */, uint16 AmountToRemove = 1);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void UseItem(TSubclassOf<AApptItem> ItemSubclass, AShopKeeper* ShopKeeper, bool IsShopItem = false);

	UFUNCTION(BlueprintNativeEvent, Category = "Inventory")
	void UpdateStats(float NewHunger, float NewHealth);

	UFUNCTION()
	void OnRep_InventoryItems();

	UFUNCTION()
	void OnRep_Stats();

	UFUNCTION(BlueprintImplementableEvent, Category="Inventory")
	void AddItemAndUpdateInventoryWidget(FItemData ItemData, const TArray<FItemData>& CurrentInventory = TArray<FItemData>());

	UFUNCTION(BlueprintImplementableEvent, Category="Inventory")
	void UpdateInventoryWidget(const TArray<FItemData>& NewInventoryItems);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	FVector CachedDestination;

	bool bIsTouch; // Is it a touch device
	float FollowTime; // For how long it has been pressed

	/** Keyboard Moving */
	void InputMove(const FInputActionValue& InputActionValue);
};


