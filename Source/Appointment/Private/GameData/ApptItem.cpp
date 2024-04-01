// Fill out your copyright notice in the Description page of Project Settings.


#include "GameData/ApptItem.h"
#include "Components/StaticMeshComponent.h"
#include "../../AppointmentPlayerController.h"

// Sets default values
AApptItem::AApptItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;	// default : true
	
	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	RootComponent = ItemMesh;

	bReplicates = true;

	ItemData.ItemClass = StaticClass();
}

// Called when the game starts or when spawned
void AApptItem::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AApptItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AApptItem::Interact(AAppointmentPlayerController* PlayerController)
{
	UE_LOG(LogTemp, Warning, TEXT("##### ApptItem::Interact() #####"));
	if (HasAuthority() && PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("##### ApptItem::Interact() - HasAuthority #####"));
		PlayerController->AddInventoryItem(ItemData);
		Destroy();
	}
}

void AApptItem::Use(AAppointmentPlayerController* PlayerController)
{
	UE_LOG(LogTemp, Warning, TEXT("##### USE ITEM : %s"), *GetName());
}

