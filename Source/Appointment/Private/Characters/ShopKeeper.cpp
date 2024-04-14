// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/ShopKeeper.h"
#include "Components/SkeletalMeshComponent.h"
#include "../AppointmentPlayerController.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AShopKeeper::AShopKeeper()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	ShopKeeperMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
	RootComponent = ShopKeeperMesh;

	bReplicates = true;
}

void AShopKeeper::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AShopKeeper, Items);
}

// Called when the game starts or when spawned
void AShopKeeper::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AShopKeeper::Tick(float DeltaTime)
{
	// Super::Tick(DeltaTime);
}

void AShopKeeper::Interact(AAppointmentPlayerController* PlayerController)
{
	// throw std::logic_error("The method or operation is not implemented.");
	if (PlayerController)
	{
		PlayerController->OpenShop(Items);
	}
}

