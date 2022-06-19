// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseBuildable.h"

ABaseBuildable::ABaseBuildable()
{
	PrimaryActorTick.bCanEverTick = false;

	Health = 100.0f;
	AssetId = -1;
	DestructibleMesh = nullptr;
	DestructibleComponent = nullptr;
}


void ABaseBuildable::BeginPlay()
{
	Super::BeginPlay();
	
}


void ABaseBuildable::SetBaseData_Implementation(UDestructibleMesh* Mesh, float MaxHealth, int Asset)
{
	DestructibleMesh = Mesh;
	Health = MaxHealth;
	AssetId = Asset;
}


void ABaseBuildable::DealDamage_Implementation(float Damage, FVector HitLocation, FVector ImpulseDir, float ImpulseStrength)
{
	if (Health > 0)
	{
		if (DestructibleComponent == nullptr)
		{
			TArray<UActorComponent*> Components = GetComponents().Array();
			FTransform Transform = GetActorTransform();

			for (int i = 0; i < Components.Num(); i++)
			{
				Components[i]->DestroyComponent();
			}

			DestructibleComponent = NewObject<UDestructibleComponent>(this, UDestructibleComponent::StaticClass());
			DestructibleComponent->SetupAttachment(RootComponent);
			DestructibleComponent->RegisterComponent();

			DestructibleComponent->SetWorldTransform(Transform);
			DestructibleComponent->SetDestructibleMesh(DestructibleMesh);

			DestructibleComponent->SetSimulatePhysics(true);
		}

		Health = FMath::Clamp(Health - Damage, 0.0f, 100.0f);
		if (Health > 0)
		{
			DamageAmount = FMath::Clamp(DamageAmount + Damage, 0.0f, 5.0f);
			DestructibleComponent->ApplyDamage(DamageAmount, HitLocation, ImpulseDir, ImpulseStrength);
		}

		for (int i = 0; i < BuildChildrens.Num(); i++)
		{
			if (BuildChildrens[i]->GetClass()->ImplementsInterface(UBuildInterface::StaticClass()))
			{
				IBuildInterface::Execute_DealDamage(BuildChildrens[i], Damage, HitLocation, ImpulseDir, ImpulseStrength);
			}
		}
	}
}

void ABaseBuildable::AddBuildChild_Implementation(AActor* Actor)
{
	BuildChildrens.Add(Actor);
}


void ABaseBuildable::SetBuildMesh_Implementation(UStaticMesh* Mesh, FTransform Transform)
{
	StaticMeshComponent = NewObject<UStaticMeshComponent>(this);

	StaticMeshComponent->SetupAttachment(RootComponent);
	StaticMeshComponent->RegisterComponent();

	StaticMeshComponent->SetStaticMesh(Mesh);
	StaticMeshComponent->SetWorldTransform(Transform);
}


TArray<UBoxComponent*> ABaseBuildable::ReturnBoxes_Implementation()
{
	return TArray<UBoxComponent*>();
}

int ABaseBuildable::GetAssetId_Implementation()
{
	return AssetId;
}