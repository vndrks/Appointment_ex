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

#include "GameData/ApptItem.h"

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
		
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &AAppointmentPlayerController::Interact);
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
		AActor* Actor = Hit.GetActor();

		if (IInteractableInterface* Interface = Cast<IInteractableInterface>(Hit.GetActor()))
		{
			UE_LOG(LogTemp, Log, TEXT("### Hit Actor(Mouse Left Click : %s"), *Actor->GetName());
			Interface->Interact(this);
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

void AAppointmentPlayerController::Interact()
{
	AAppointmentCharacter* ApptCharacter = Cast<AAppointmentCharacter>(GetPawn());
	// UE_LOG(LogTemp, Warning, TEXT("##### Interact()"));

	if (ApptCharacter)
	{
		FVector StartLocation = ApptCharacter->GetActorLocation();
		// FVector ForwardVector = StartLocation + ApptCharacter->GetActorForwardVector() + 200.f;
		FVector ForwardVector = StartLocation + FVector(200.f, 200.f, 200.f);
		FVector BottomLocation = ApptCharacter->GetActorForwardVector() + FVector(200.f, StartLocation.Y, StartLocation.Z);
		
		// UE_LOG(LogTemp, Warning, TEXT("##### ForwardVector.Z : %f"), ForwardVector.Z);

		float ZMin = ForwardVector.Z - 100.f;
		float ZMax = ForwardVector.Z + 100.f;

		//UCameraComponent* CameraComponent = ApptCharacter->GetTopDownCameraComponent();
		//FVector Start = CameraComponent->GetComponentLocation();
		//FVector End = CameraComponent->GetComponentLocation() * 5000.0f;

		FHitResult HitResult;
		FCollisionQueryParams CollisionQueryParams;
		CollisionQueryParams.AddIgnoredActor(this);
		CollisionQueryParams.TraceTag = FName("ZRangeTrace");

		bool IsHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, BottomLocation, ECC_Visibility, CollisionQueryParams);
		AActor* ActorTemp = HitResult.GetActor();
		if (IsHit)
		{
			// UE_LOG(LogTemp, Warning, TEXT("##### Hit Actor 1 : %s"), *ActorTemp->GetName());
			if (IInteractableInterface* Interface = Cast<IInteractableInterface>(HitResult.GetActor()))
			{
				//UE_LOG(LogTemp, Warning, TEXT("##### Hit Actor 2"));
				Interface->Interact(this);
			}

			//if (HitResult.ImpactPoint.Z >= ZMin && HitResult.ImpactPoint.Z <= ZMax)
			//{	
			//	if (IInteractableInterface* Interface = Cast<IInteractableInterface>(HitResult.GetActor()))
			//	{
			//		UE_LOG(LogTemp, Warning, TEXT("##### Hit Actor 2"));
			//		Interface->Interact();
			//	}
			//	/*
			//	if (AActor* Actor = HitResult.GetActor())
			//	{
			//		UE_LOG(LogTemp, Warning, TEXT("Hit Actor : %s"), *Actor->GetName());
			//	}
			//	*/
			//}
		}
		else
		{
			if (ActorTemp)
				DrawDebugLine(ActorTemp->GetWorld(), StartLocation, BottomLocation, FColor::Green, false, 0.1f, 0, 5.f);
		}
	}
}

void AAppointmentPlayerController::UseItem(TSubclassOf<AApptItem> ItemSubclass)
{
	if (ItemSubclass)
	{
		if (AApptItem* Item = ItemSubclass.GetDefaultObject())
		{
			Item->Use(this);
		}			
	}
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
