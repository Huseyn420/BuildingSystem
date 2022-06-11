// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildManagerComponent.h"
#include "BuildingSystemCharacter.h"
#include "BuildInterface.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Camera/CameraComponent.h"

UBuildManagerComponent::UBuildManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	static ConstructorHelpers::FObjectFinder<UMaterial> AssetGreenColor(TEXT("Material'/Game/Materials/M_GreenColor.M_GreenColor'"));
	if (AssetGreenColor.Succeeded())
	{
		GreenColor = AssetGreenColor.Object;
	}

	static ConstructorHelpers::FObjectFinder<UMaterial> AssetRedColor(TEXT("Material'/Game/Materials/M_RedColor.M_RedColor'"));
	if (AssetGreenColor.Succeeded())
	{
		RedColor = AssetRedColor.Object;
	}

	bCanBuild = false;
	bIsBuildModeOn = false;
	BuildId = 0;
	BuildGhostRotator = FRotator(0, 90, 0);
}


void UBuildManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	UDataTable* Table = LoadObject<UDataTable>(NULL, TEXT("DataTable'/Game/Database/DT_Buildables.DT_Buildables'"));
	Table->GetAllRows(FString("Buildables"), Buildables);

	PlayerRef = Cast<ABuildingSystemCharacter>(GetOwner());
}


void UBuildManagerComponent::LaunchBuildMode()
{
	check(Scene != nullptr && PlayerRef != nullptr && Camera != nullptr);

	bIsBuildModeOn = !bIsBuildModeOn;
	bIsBuildModeOn ? BuildDelay() : DestroyBuildGhost();
}


void UBuildManagerComponent::BuildDelay()
{
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [&]()
	{
		bIsBuildModeOn ? BuildCycle() : TimerHandle.Invalidate();
	}, 0.01, true);
}


void UBuildManagerComponent::BuildCycle()
{
	FVector LineStart = PlayerRef->GetActorLocation() + Camera->GetForwardVector() * 100.0f;
	FVector LineEnd = PlayerRef->GetActorLocation() + Camera->GetForwardVector() * 1000.0f;

	if (!BuildGhost)
	{
		SpawnBuildGhost();
	}

	FHitResult TraceResult;
	ETraceTypeQuery TraceChannel = Buildables[BuildId]->TraceChannel.GetValue();
	ECollisionChannel CollisionChannel = UEngineTypes::ConvertToCollisionChannel(TraceChannel);

	if (GetWorld()->LineTraceSingleByChannel(TraceResult, LineStart, LineEnd, CollisionChannel))
	{
		AActor* Actor = TraceResult.GetActor();
		UPrimitiveComponent* Component = TraceResult.GetComponent();
		if (DetectBuildBoxed(Actor, Component))
		{
			FTransform Transform = Component->GetComponentTransform();
			BuildGhost->SetWorldTransform(Transform);
			HitActor = Actor;
		}
		else
		{
			FTransform Transform = FTransform(BuildGhostRotator, TraceResult.ImpactPoint);
			BuildGhost->SetWorldTransform(Transform);
			HitActor = nullptr;
		}
	}
	else
	{
		FTransform Transform = FTransform(BuildGhostRotator, TraceResult.TraceEnd);
		BuildGhost->SetWorldTransform(Transform);
		HitActor = nullptr;
	}

	bCanBuild = !(CheckForOverlap()) && IsBuildFloating();
	GiveBuildColor(bCanBuild);
}


void UBuildManagerComponent::SpawnBuildGhost()
{
	check(BuildGhost == nullptr);

	BuildGhost = NewObject<UStaticMeshComponent>(this);

	BuildGhost->SetupAttachment(Scene);
	BuildGhost->RegisterComponent();

	UpdateMesh();
	BuildGhost->SetMobility(EComponentMobility::Movable);
	BuildGhost->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}


void UBuildManagerComponent::GiveBuildColor(bool bIsGreen)
{
	for (int i = 0; i < BuildGhost->GetNumMaterials(); i++)
	{
		UMaterial* Color = bIsGreen ? GreenColor : RedColor;
		BuildGhost->SetMaterial(i, Color);
	}
}


void UBuildManagerComponent::UpdateMesh()
{
	check(Buildables.IsValidIndex(BuildId));

	if (bIsBuildModeOn && BuildGhost != nullptr)
	{
		TAssetPtr<UStaticMesh> BaseMesh = Buildables[BuildId]->Mesh;
		if (BaseMesh.IsValid())
		{
			BuildGhost->SetStaticMesh(BaseMesh.Get());
		}
		else if (BaseMesh.IsPending())
		{
			const FSoftObjectPath& AssetRef = BaseMesh.ToSoftObjectPath();
			UStaticMesh* Mesh = Cast<UStaticMesh>(UAssetManager::GetStreamableManager().LoadSynchronous(AssetRef));
			BuildGhost->SetStaticMesh(Mesh);
		}
	}
}


void UBuildManagerComponent::DealDamage()
{
	check(Buildables.IsValidIndex(BuildId));

	if (!bIsBuildModeOn)
	{
		FVector LineStart = PlayerRef->GetActorLocation() + Camera->GetForwardVector();
		FVector LineEnd = PlayerRef->GetActorLocation() + Camera->GetForwardVector() * 2000.0f;

		FHitResult TraceResult;
		if (GetWorld()->LineTraceSingleByChannel(TraceResult, LineStart, LineEnd, ECC_Visibility))
		{
			AActor* Actor = TraceResult.GetActor();
			if (Actor->GetClass()->ImplementsInterface(UBuildInterface::StaticClass()))
			{
				float Damage = 100.0f;
				FVector HitLocation = TraceResult.Location;
				FVector ImpulseDir = Camera->GetForwardVector();
				float ImpulseStrength = 100.0f;
				IBuildInterface::Execute_DealDamage(Actor, Damage, HitLocation, ImpulseDir, ImpulseStrength);
			}
		}
	}
}


void UBuildManagerComponent::SpawnBuild()
{
	check(Buildables.IsValidIndex(BuildId));

	if (bIsBuildModeOn && bCanBuild)
	{
		FTransform Transform = BuildGhost->GetComponentToWorld();
		AActor* Actor = GetWorld()->SpawnActor(Buildables[BuildId]->Actor, &Transform);
		int GroupId = Buildables[BuildId]->GroupId;
		float Health = Buildables[BuildId]->Health;

		if (Actor->GetClass()->ImplementsInterface(UBuildInterface::StaticClass()))
		{
			TAssetPtr<UDestructibleMesh> DestructibleMesh = Buildables[BuildId]->DestructibleMesh;
			if (DestructibleMesh.IsValid())
			{
				UDestructibleMesh* Mesh = DestructibleMesh.Get();
				IBuildInterface::Execute_SetBaseData(Actor, Mesh, Health, GroupId);
			}
			else if (DestructibleMesh.IsPending())
			{
				const FSoftObjectPath& AssetRef = DestructibleMesh.ToSoftObjectPath();
				UDestructibleMesh* Mesh = Cast<UDestructibleMesh>(UAssetManager::GetStreamableManager().LoadSynchronous(AssetRef));
				IBuildInterface::Execute_SetBaseData(Actor, Mesh, Health, GroupId);
			}
		}

		if (HitActor != nullptr && HitActor->GetClass()->ImplementsInterface(UBuildInterface::StaticClass()))
		{
			IBuildInterface::Execute_AddBuildChild(HitActor, Actor);
		}
	}
}


bool UBuildManagerComponent::IsBuildFloating()
{
	check(BuildGhost != nullptr);

	FVector LineStart = BuildGhost->GetComponentLocation();
	FVector LineEnd = LineStart + FVector(0, 0, -1000.0f);

	FHitResult TraceResult;
	GetWorld()->LineTraceSingleByChannel(TraceResult, LineStart, LineEnd, ECC_Visibility);

	return TraceResult.bBlockingHit;
}


bool UBuildManagerComponent::CheckForOverlap()
{
	check(BuildGhost != nullptr);

	FHitResult SweepResult;
	FBoxSphereBounds Bounds = BuildGhost->CalcLocalBounds();
	FRotator Rotator = BuildGhost->GetComponentRotation();
	FVector Start = BuildGhost->GetComponentToWorld().GetLocation() + FVector(0, 0, Bounds.GetBox().GetSize().Z / 2);
	FVector End = Start + 1.0f;

	UKismetSystemLibrary::BoxTraceSingle(GetWorld(), Start, End, Bounds.BoxExtent * 0.8, Rotator, 
										 ETraceTypeQuery::TraceTypeQuery1, true, TArray<AActor*>(), 
										 EDrawDebugTrace::None, SweepResult, true);

	return SweepResult.bBlockingHit;
}


void UBuildManagerComponent::DestroyBuildGhost()
{
	BuildGhost->DestroyComponent();
	BuildGhost = nullptr;
}


void UBuildManagerComponent::InteractWithBuild()
{
	FVector LineStart = PlayerRef->GetActorLocation() + Camera->GetForwardVector();
	FVector LineEnd = PlayerRef->GetActorLocation() + Camera->GetForwardVector() * 2000.0f;

	FHitResult TraceResult;
	if (GetWorld()->LineTraceSingleByChannel(TraceResult, LineStart, LineEnd, ECC_Visibility))
	{
		AActor* Actor = TraceResult.GetActor();
		if (Actor->GetClass()->ImplementsInterface(UBuildInterface::StaticClass()))
		{
			IBuildInterface::Execute_InteractWithBuild(Actor);
		}
	}
}


bool UBuildManagerComponent::DetectBuildBoxed(AActor* Actor, UPrimitiveComponent* Component)
{
	if (Actor->GetClass()->ImplementsInterface(UBuildInterface::StaticClass()))
	{
		TArray<UBoxComponent*> Boxes = IBuildInterface::Execute_ReturnBoxes(Actor);
		for (int i = 0; i < Boxes.Num(); i++)
		{
			if (Component == Boxes[i])
			{
				return true;
			}
		}
	}

	return false;
}


void UBuildManagerComponent::RotateBuildGhost(float Value)
{
	if (bIsBuildModeOn && BuildGhost)
	{
		BuildGhostRotator.Yaw += round(Value);
		BuildGhost->SetWorldRotation(BuildGhostRotator);
	}
}