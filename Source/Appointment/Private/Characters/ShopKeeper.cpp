// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/ShopKeeper.h"
#include "Components/SkeletalMeshComponent.h"
#include "../AppointmentPlayerController.h"
#include "../AppointmentCharacter.h"
#include "Kismet/GameplayStatics.h"
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

void AShopKeeper::OnRep_Items()
{
	if (AAppointmentPlayerController* PlayerController = Cast<AAppointmentPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0)))
	{
		if (PlayerController->GetCharacter()->IsLocallyControlled())
		{
			PlayerController->OpenShop(this, Items);
		}
	}
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

void AShopKeeper::TransfferedItem(TSubclassOf<AApptItem> ItemSubclass)
{
	// throw std::logic_error("The method or operation is not implemented.");
	uint8 Index = 0;
	for (FItemData& Item : Items)
	{
		if (Item.ItemClass == ItemSubclass)
		{
			--Item.StackCount;
			if (Item.StackCount <= 0)
			{
				Items.RemoveAt(Index);
			}
			break;
		}
		++Index;
	}

	for (FItemData& Item : Items)
	{
		UE_LOG(LogTemp, Warning, TEXT("##### Stack Count : %d"), Item.StackCount);
	}
	OnRep_Items();
}

bool AShopKeeper::CanBuyItem(int32 CurrentGold, TSubclassOf<AApptItem> ItemSubclass)
{
	for (FItemData& Item : Items)
	{
		if (Item.ItemClass == ItemSubclass)
		{
			return CurrentGold >= Item.ItemCost;
		}
	}

	return false;
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
		PlayerController->OpenShop(this, Items);
	}
}

bool AShopKeeper::BuyItem(class AAppointmentPlayerController* PlayerController, TSubclassOf<AApptItem> ItemSubclass)
{
	if (PlayerController && ItemSubclass)
	{
		for (const FItemData& Item : Items)
		{
			if (Item.ItemClass == ItemSubclass)
			{
				if (CanBuyItem(PlayerController->GetCurrentGold(), ItemSubclass))
				{
					if (AApptItem* ItemCDO = ItemSubclass.GetDefaultObject())
					{
						ItemCDO->Use(PlayerController, true);
						PlayerController->RemoveGold(Item.ItemCost);
						TransfferedItem(ItemSubclass);
						return true;
					}
				}
			}
		}
	}
	return false;
}

