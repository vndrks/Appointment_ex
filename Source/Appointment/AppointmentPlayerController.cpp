// Copyright Epic Games, Inc. All Rights Reserved.

#include "AppointmentPlayerController.h"
#include "GameFramework/Pawn.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "AppointmentCharacter.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Net/UnrealNetwork.h"

#include "GameData/ApptItem.h"
#include "GameData/Gold.h"
#include "Characters/ShopKeeper.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

AAppointmentPlayerController::AAppointmentPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
	CachedDestination = FVector::ZeroVector;
	FollowTime = 0.f;
}

void AAppointmentPlayerController::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}
}

void AAppointmentPlayerController::AddInventoryItem(FItemData ItemData)
{
	AAppointmentCharacter* PlayerCharacter = Cast<AAppointmentCharacter>(GetPawn());

	if (PlayerCharacter->HasAuthority())
	{
		bool bIsNewItem = true;
		
		for (FItemData& Item : InventoryItems)
		{
			if (Item.ItemClass == ItemData.ItemClass)
			{
				if (ItemData.StackCount > 1)
				{
					Item.StackCount += ItemData.StackCount;
				}
				else
				{
					++Item.StackCount;
				}
				bIsNewItem = false;
				break;
			}
		}

		if (bIsNewItem)
		{
			InventoryItems.Add(ItemData);
		}
		if (PlayerCharacter->IsLocallyControlled())
		{
			OnRep_InventoryItems();
		}
	}
}

void AAppointmentPlayerController::AddHealth(float Value)
{
	AAppointmentCharacter* PlayerCharacter = Cast<AAppointmentCharacter>(GetPawn());
	Health += Value;

	if (PlayerCharacter->IsLocallyControlled())
	{
		UpdateStats(Hunger, Health);
	}
}

void AAppointmentPlayerController::RemoveHunger(float Value)
{
	AAppointmentCharacter* PlayerCharacter = Cast<AAppointmentCharacter>(GetPawn());
	Hunger -= Value;

	if (PlayerCharacter->IsLocallyControlled())
	{
		UpdateStats(Hunger, Health);
	}
}

int32 AAppointmentPlayerController::GetCurrentGold()
{
	for (FItemData& Item : InventoryItems)
	{
		if (Item.ItemClass->StaticClass() == TSubclassOf<AGold>()->StaticClass())
		{
			UE_LOG(LogTemp, Warning, TEXT("##### STACK COUNT : %d"), Item.StackCount);
			return Item.StackCount;
		}
	}
	return 0;
}

void AAppointmentPlayerController::RemoveGold(int32 AmountToRemove)
{
	for (FItemData& Item : InventoryItems)
	{
		if (Item.ItemClass->StaticClass() == TSubclassOf<AGold>()->StaticClass())
		{
			Item.StackCount -= AmountToRemove;
		}
	}
}

void AAppointmentPlayerController::SetupInputComponent()
{
	// set up gameplay key bindings
	Super::SetupInputComponent();

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		// Setup mouse input events
		EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Started, this, &AAppointmentPlayerController::OnInputStarted);
		EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Triggered, this, &AAppointmentPlayerController::OnSetDestinationTriggered);
		EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Completed, this, &AAppointmentPlayerController::OnSetDestinationReleased);
		EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Canceled, this, &AAppointmentPlayerController::OnSetDestinationReleased);

		// Setup touch input events
		EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Started, this, &AAppointmentPlayerController::OnInputStarted);
		EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Triggered, this, &AAppointmentPlayerController::OnTouchTriggered);
		EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Completed, this, &AAppointmentPlayerController::OnTouchReleased);
		EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Canceled, this, &AAppointmentPlayerController::OnTouchReleased);

		// Setup custom event by Caspar
		EnhancedInputComponent->BindAction(SetKeyboardMoveAction, ETriggerEvent::Started, this, &AAppointmentPlayerController::OnInputStarted);
		EnhancedInputComponent->BindAction(SetKeyboardMoveAction, ETriggerEvent::Triggered, this, &AAppointmentPlayerController::InputMove);
		
		// EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &AAppointmentPlayerController::Interact);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AAppointmentPlayerController::OnInputStarted()
{
	StopMovement();
}

// Triggered every frame when the input is held down
void AAppointmentPlayerController::OnSetDestinationTriggered()
{
	// We flag that the input is being pressed
	FollowTime += GetWorld()->GetDeltaSeconds();
	
	// We look for the location in the world where the player has pressed the input
	FHitResult Hit;
	bool bHitSuccessful = false;
	if (bIsTouch)
	{
		bHitSuccessful = GetHitResultUnderFinger(ETouchIndex::Touch1, ECollisionChannel::ECC_Visibility, true, Hit);
	}
	else
	{
		bHitSuccessful = GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, true, Hit);
	}

	// If we hit a surface, cache the location
	if (bHitSuccessful)
	{
		CachedDestination = Hit.Location;

		/** Interact Code (Temporary) : Caspar */
		AActor* HitActor = Hit.GetActor();

		if (AAppointmentCharacter* PlayerCharacter = Cast<AAppointmentCharacter>(GetPawn()))
		{
			FVector Start = PlayerCharacter->GetActorLocation();
			FVector End = Start + PlayerCharacter->GetActorForwardVector();

			if (PlayerCharacter->HasAuthority())
			{
				Interact(Start, End, HitActor);
			}
			else
			{
				Interact(Start, End, HitActor);
				Server_Interact(Start, End, HitActor);
			}
		}		
	}
	
	// Move towards mouse pointer or touch
	APawn* ControlledPawn = GetPawn();
	if (ControlledPawn != nullptr)
	{
		FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();
		ControlledPawn->AddMovementInput(WorldDirection, 1.0, false);
	}
}

void AAppointmentPlayerController::OnSetDestinationReleased()
{
	// If it was a short press
	if (FollowTime <= ShortPressThreshold)
	{
		// We move there and spawn some particles
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, CachedDestination);
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FXCursor, CachedDestination, FRotator::ZeroRotator, FVector(1.f, 1.f, 1.f), true, true, ENCPoolMethod::None, true);
	}

	FollowTime = 0.f;
}

// Triggered every frame when the input is held down
void AAppointmentPlayerController::OnTouchTriggered()
{
	bIsTouch = true;
	OnSetDestinationTriggered();
}

void AAppointmentPlayerController::OnTouchReleased()
{
	bIsTouch = false;
	OnSetDestinationReleased();
}

bool AAppointmentPlayerController::Server_Interact_Validate(FVector Start, FVector End, AActor* HitActor)
{
	return true;
}

void AAppointmentPlayerController::Server_Interact_Implementation(FVector Start, FVector End, AActor* HitActor)
{
	Interact(Start, End, HitActor);
}

//void AAppointmentPlayerController::Interact()
//{
//	AAppointmentCharacter* ApptCharacter = Cast<AAppointmentCharacter>(GetPawn());
//	UE_LOG(LogTemp, Warning, TEXT("##### Interact()"));
//
//	if (ApptCharacter)
//	{
//		FVector StartLocation = ApptCharacter->GetActorLocation();
//		// FVector ForwardVector = StartLocation + ApptCharacter->GetActorForwardVector() + 200.f;
//		FVector ForwardVector = StartLocation + FVector(200.f, 200.f, 200.f);
//		FVector BottomLocation = ApptCharacter->GetActorForwardVector() + FVector(200.f, StartLocation.Y, StartLocation.Z);
//		
//		// UE_LOG(LogTemp, Warning, TEXT("##### ForwardVector.Z : %f"), ForwardVector.Z);
//
//		float ZMin = ForwardVector.Z - 100.f;
//		float ZMax = ForwardVector.Z + 100.f;
//
//		//UCameraComponent* CameraComponent = ApptCharacter->GetTopDownCameraComponent();
//		//FVector Start = CameraComponent->GetComponentLocation();
//		//FVector End = CameraComponent->GetComponentLocation() * 5000.0f;
//		/**/
//		FHitResult HitResult;
//		FCollisionQueryParams CollisionQueryParams;
//		CollisionQueryParams.AddIgnoredActor(this);
//		CollisionQueryParams.TraceTag = FName("ZRangeTrace");
//
//		bool IsHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, BottomLocation, ECC_Visibility, CollisionQueryParams);
//		AActor* ActorTemp = HitResult.GetActor();
//		if (IsHit)
//		{
//			// UE_LOG(LogTemp, Warning, TEXT("##### Hit Actor 1 : %s"), *ActorTemp->GetName());
//			if (IInteractableInterface* Interface = Cast<IInteractableInterface>(HitResult.GetActor()))
//			{
//				//UE_LOG(LogTemp, Warning, TEXT("##### Hit Actor 2"));
//				Interface->Interact(this);
//			}
//
//			//if (HitResult.ImpactPoint.Z >= ZMin && HitResult.ImpactPoint.Z <= ZMax)
//			//{	
//			//	if (IInteractableInterface* Interface = Cast<IInteractableInterface>(HitResult.GetActor()))
//			//	{
//			//		UE_LOG(LogTemp, Warning, TEXT("##### Hit Actor 2"));
//			//		Interface->Interact();
//			//	}
//			//	/*
//			//	if (AActor* Actor = HitResult.GetActor())
//			//	{
//			//		UE_LOG(LogTemp, Warning, TEXT("Hit Actor : %s"), *Actor->GetName());
//			//	}
//			//	*/
//			//}
//		}
//		else
//		{
//			if (ActorTemp)
//				DrawDebugLine(ActorTemp->GetWorld(), StartLocation, BottomLocation, FColor::Green, false, 0.1f, 0, 5.f);
//		}
//	}
//}

void AAppointmentPlayerController::Interact(FVector Start, FVector End, AActor* HitActor)
{
	AShopKeeper* ShopKeeper = Cast<AShopKeeper>(HitActor);
	if (ShopKeeper)
	{
		UE_LOG(LogTemp, Warning, TEXT("##### OPENING SHOPKEEPER"));
		if (GetCharacter()->IsLocallyControlled())
		{
			ShopKeeper->Interact(this);
		}
		return;
	}

	if (IInteractableInterface* Interface = Cast<IInteractableInterface>(HitActor))
	{
		Interface->Interact(this);
	}

	//FHitResult HitResult;
	//FCollisionQueryParams Params;
	//Params.AddIgnoredActor(this);

	//if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params))
	//{
	//	if (IInteractableInterface* Interface = Cast<IInteractableInterface>(HitResult.GetActor()))
	//	{
	//		Interface->Interact(this);
	//	}
	//}
}

bool AAppointmentPlayerController::Server_UseItem_Validate(TSubclassOf<AApptItem> ItemSubclass, AShopKeeper* ShopKeeper, bool IsShopItem)
{
	return true;
}

void AAppointmentPlayerController::Server_UseItem_Implementation(TSubclassOf<AApptItem> ItemSubclass, AShopKeeper* ShopKeeper, bool IsShopItem)
{
	if (IsShopItem)
	{
		UseItem(ItemSubclass, ShopKeeper, IsShopItem);
	}
	else
	{
		for (const auto& Item : InventoryItems)
		{
			if (Item.ItemClass == ItemSubclass)
			{
				if (Item.StackCount)
				{
					UseItem(ItemSubclass, ShopKeeper, IsShopItem);
				}
				return;
			}
		}
	}

}

void AAppointmentPlayerController::UseRemoveItem(TSubclassOf<AApptItem> ItemSubclass)
{
	uint8 Index = 0;
	for (FItemData& Item : InventoryItems)
	{
		if (Item.ItemClass == ItemSubclass)
		{
			if (AApptItem* ItemCDO = ItemSubclass.GetDefaultObject())
			{
				ItemCDO->Use(this, false);
			}
			--Item.StackCount;
			if (Item.StackCount <= 0)
			{
				InventoryItems.RemoveAt(Index);
			}
			break;
		}
		++Index;
	}
	AAppointmentCharacter* PlayerCharacter = Cast<AAppointmentCharacter>(GetPawn());
	if (PlayerCharacter->IsLocallyControlled())
	{
		OnRep_InventoryItems();
	}
}

void AAppointmentPlayerController::UseItem(TSubclassOf<AApptItem> ItemSubclass, AShopKeeper* ShopKeeper, bool IsShopItem)
{
	if (ItemSubclass)
	{
		if (HasAuthority())
		{
			if (!ShopKeeper)
			{
				UseRemoveItem(ItemSubclass);
			}
			else
			{
				ShopKeeper->BuyItem(this, ItemSubclass);

				if (GetCharacter()->IsLocallyControlled())
				{
					OnRep_InventoryItems();
				}
			}
		}
		else
		{
			if (AApptItem* Item = ItemSubclass.GetDefaultObject())
			{
				Item->Use(this, IsShopItem);
			}
			Server_UseItem(ItemSubclass, ShopKeeper, IsShopItem);
		}
	}
}

void AAppointmentPlayerController::OnRep_InventoryItems()
{
	if (InventoryItems.Num())
	{
		AddItemAndUpdateInventoryWidget(InventoryItems[InventoryItems.Num() - 1], InventoryItems);
	}
	else
	{
		AddItemAndUpdateInventoryWidget(FItemData(), InventoryItems);
	}
}

void AAppointmentPlayerController::OnRep_Stats()
{
	AAppointmentCharacter* PlayerCharacter = Cast<AAppointmentCharacter>(GetPawn());
	if (PlayerCharacter->IsLocallyControlled())
	{
		UpdateStats(Hunger, Health);
	}
}

void AAppointmentPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(AAppointmentPlayerController, InventoryItems, COND_OwnerOnly);
	DOREPLIFETIME(AAppointmentPlayerController, Hunger);
	DOREPLIFETIME(AAppointmentPlayerController, Health);
}

void AAppointmentPlayerController::UpdateStats_Implementation(float NewHunger, float NewHealth)
{
	
}

void AAppointmentPlayerController::InputMove(const FInputActionValue& InputActionValue)
{
	const FVector2D value = InputActionValue.Get<FVector2D>();
	const FRotator MovementRotation(0.0f, GetControlRotation().Yaw, 0.0f);

	if (value.X != 0.0f)
	{
		const FVector MovementDirection = MovementRotation.RotateVector(FVector::RightVector);
		GetPawn()->AddMovementInput(MovementDirection, value.X);
	}

	if (value.Y != 0.0f)
	{
		const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
		GetPawn()->AddMovementInput(MovementDirection, value.Y);
	}
}
