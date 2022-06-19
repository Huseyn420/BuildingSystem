// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildManagerComponent.h"
#include "BuildingSystemCharacter.h"
#include "BuildingSystem/BaseBuildable.h"
#include "Interfaces/BuildInterface.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Camera/CameraComponent.h"
#include "Widgets/BuildingMenu.h"

UBuildManagerComponent::UBuildManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	static ConstructorHelpers::FObjectFinder<UMaterial> AssetGreenColor(TEXT("Material'/Game/BuildingSystem/Materials/M_GreenColor.M_GreenColor'"));
	if (AssetGreenColor.Succeeded())
	{
		GreenColor = AssetGreenColor.Object;
	}

	static ConstructorHelpers::FObjectFinder<UMaterial> AssetRedColor(TEXT("Material'/Game/BuildingSystem/Materials/M_RedColor.M_RedColor'"));
	if (AssetGreenColor.Succeeded())
	{
		RedColor = AssetRedColor.Object;
	}

	bCanBuild = false;
	bIsBuildModeOn = false;
	bWidgetOpen = false;
	BuildId = 0;
	BuildGhostOffset = FVector(0.0f);
	BuildGhostRotator = FRotator(0.0f, 90.0f, 0.0f);
}


void UBuildManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	TArray<FTableInfo*> OutTables;
	UDataTable* MainTable = LoadObject<UDataTable>(NULL, TEXT("DataTable'/Game/BuildingSystem/DataTables/DT_MainTable.DT_MainTable'"));
	
	MainTable->GetAllRows(FString("MainTable"), OutTables);
	for (const auto& Table : OutTables)
	{
		Tables.Emplace(*Table);
	}
	LoadBuildables(0);

	PlayerRef = Cast<ABuildingSystemCharacter>(GetOwner());

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PlayerController)
	{
		BuildingMenu = CreateWidget<UBuildingMenu>(PlayerController, UBuildingMenu::StaticClass());
		BuildingMenu->SetData(this, &Buildables, BuildId, Group);
	}
}


void UBuildManagerComponent::LaunchBuildMode()
{
	check(Scene != nullptr && PlayerRef != nullptr && Camera != nullptr);

	if (!bWidgetOpen)
	{
		bIsBuildModeOn = !bIsBuildModeOn;
		bIsBuildModeOn ? BuildDelay() : DestroyBuildGhost();
	}
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
	if (Buildables.IsValidIndex(BuildId))
	{
		FVector LineStart = Camera->GetComponentLocation() + Camera->GetForwardVector() * 400.0f;
		FVector LineEnd = Camera->GetComponentLocation() + Camera->GetForwardVector() * 1000.0f;

		if (!BuildGhost)
		{
			SpawnBuildGhost();
		}

		FHitResult TraceResult;
		ETraceTypeQuery TraceChannel = Buildables[BuildId].TraceChannel.GetValue();
		ECollisionChannel CollisionChannel = UEngineTypes::ConvertToCollisionChannel(TraceChannel);

		if (GetWorld()->LineTraceSingleByChannel(TraceResult, LineStart, LineEnd, CollisionChannel))
		{
			AActor* Actor = TraceResult.GetActor();
			UPrimitiveComponent* Component = TraceResult.GetComponent();
			if (DetectBuildBoxed(Actor, Component))
			{
				FTransform Transform = Component->GetComponentTransform();
				if (HitActor)
				{
					FRotator Rotator = Transform.Rotator();
					Rotator.Yaw += 90.0f * floor(BuildGhostRotator.Yaw / 90.0f);
					Transform.SetRotation(FQuat(Rotator));
					Transform.SetLocation(Transform.GetLocation() + BuildGhostOffset);
				}
				else
				{
					BuildGhostOffset = FVector(0.0f);
					BuildGhostRotator = FRotator(0.0f);
				}
				BuildGhost->SetWorldTransform(Transform);
				HitActor = Actor;
			}
			else
			{
				FTransform Transform = FTransform(BuildGhostRotator, TraceResult.ImpactPoint + BuildGhostOffset);
				BuildGhost->SetWorldTransform(Transform);
				HitActor = nullptr;
			}
		}
		else
		{
			FTransform Transform = FTransform(BuildGhostRotator, TraceResult.TraceEnd + BuildGhostOffset);
			BuildGhost->SetWorldTransform(Transform);
			HitActor = nullptr;
		}

		bCanBuild = !(CheckForOverlap()) && IsBuildFloating();
		GiveBuildColor(bCanBuild);
	}
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


void UBuildManagerComponent::DealDamage()
{
	if (!bIsBuildModeOn && Buildables.IsValidIndex(BuildId) && !bWidgetOpen)
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


void UBuildManagerComponent::UpdateMesh()
{
	if (UStaticMesh* Mesh = GetBuildMesh())
	{
		BuildGhost->SetStaticMesh(Mesh);
		BuildingMenu->SetBuildId(BuildId);
	}
}


UStaticMesh* UBuildManagerComponent::GetBuildMesh()
{
	if (bIsBuildModeOn && BuildGhost != nullptr && Buildables.IsValidIndex(BuildId))
	{
		TAssetPtr<UStaticMesh> BaseMesh = Buildables[BuildId].Mesh;
		if (BaseMesh.IsValid())
		{
			return BaseMesh.Get();
		}
		else if (BaseMesh.IsPending())
		{
			const FSoftObjectPath& AssetRef = BaseMesh.ToSoftObjectPath();
			UStaticMesh* Mesh = Cast<UStaticMesh>(UAssetManager::GetStreamableManager().LoadSynchronous(AssetRef));
			return Mesh;
		}
	}

	return nullptr;
}


AActor* UBuildManagerComponent::CreateBuildActor(FTransform Transform)
{
	if (Buildables.IsValidIndex(BuildId))
	{
		if (TSubclassOf<AActor> Actor = Buildables[BuildId].Actor)
		{
			return GetWorld()->SpawnActor(Actor, &Transform);
		}
		else if (UStaticMesh* Mesh = GetBuildMesh())
		{
			ABaseBuildable* BaseBuildable = GetWorld()->SpawnActor<ABaseBuildable>();
			BaseBuildable->Execute_SetBuildMesh(BaseBuildable, Mesh, Transform);
			return BaseBuildable;
		}
	}

	return nullptr;
}


void UBuildManagerComponent::SpawnBuild()
{
	if (bIsBuildModeOn && bCanBuild && !bWidgetOpen)
	{
		FTransform Transform = BuildGhost->GetComponentToWorld();
		if (AActor* Actor = CreateBuildActor(Transform))
		{
			int AssetId = Buildables[BuildId].AssetId;
			float Health = Buildables[BuildId].Health;

			if (Actor->GetClass()->ImplementsInterface(UBuildInterface::StaticClass()))
			{
				TAssetPtr<UDestructibleMesh> DestructibleMesh = Buildables[BuildId].DestructibleMesh;
				if (DestructibleMesh.IsValid())
				{
					UDestructibleMesh* Mesh = DestructibleMesh.Get();
					IBuildInterface::Execute_SetBaseData(Actor, Mesh, Health, AssetId);
				}
				else if (DestructibleMesh.IsPending())
				{
					const FSoftObjectPath& AssetRef = DestructibleMesh.ToSoftObjectPath();
					UDestructibleMesh* Mesh = Cast<UDestructibleMesh>(UAssetManager::GetStreamableManager().LoadSynchronous(AssetRef));
					IBuildInterface::Execute_SetBaseData(Actor, Mesh, Health, AssetId);
				}
			}

			if (HitActor != nullptr && HitActor->GetClass()->ImplementsInterface(UBuildInterface::StaticClass()))
			{
				IBuildInterface::Execute_AddBuildChild(HitActor, Actor);
			}
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

	if (Buildables.IsValidIndex(BuildId))
	{
		FHitResult SweepResult;
		FBoxSphereBounds Bounds = BuildGhost->CalcLocalBounds();
		FRotator Rotator = BuildGhost->GetComponentRotation();
		float Offset = Buildables[BuildId].BoxCollisionOffset;
		float SizeFactor = Buildables[BuildId].BoxCollisionSizeFactor;
		FVector Start = BuildGhost->GetComponentToWorld().GetLocation() + FVector(0, 0, Bounds.GetBox().GetSize().Z / 2) * Offset;
		FVector End = Start + 1.0f;

		UKismetSystemLibrary::BoxTraceSingle(GetWorld(), Start, End, Bounds.BoxExtent * SizeFactor, Rotator, 
											 ETraceTypeQuery::TraceTypeQuery1, true, TArray<AActor*>(), 
											 EDrawDebugTrace::None, SweepResult, true);

		return SweepResult.bBlockingHit;
	}

	return true;
}


void UBuildManagerComponent::DestroyBuildGhost()
{
	if (BuildGhost && !bWidgetOpen)
	{
		BuildGhost->DestroyComponent();
		BuildGhost = nullptr;
	}
}


void UBuildManagerComponent::OpenBuildingMenu_Implementation()
{
	if (!BuildingMenu->IsInViewport() && bIsBuildModeOn)
	{
		APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		FInputModeGameAndUI InputModeData;
		
		InputModeData.SetWidgetToFocus(BuildingMenu->TakeWidget());
		InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

		PlayerController->SetInputMode(InputModeData);
		PlayerController->bShowMouseCursor = true;

		BuildingMenu->AddToViewport();
		bWidgetOpen = true;
	}
	else
	{
		CloseBuildingMenu_Implementation(BuildId);
	}
}


void UBuildManagerComponent::CloseBuildingMenu_Implementation(int NewBuildId)
{
	if (BuildingMenu->IsInViewport() && bIsBuildModeOn)
	{
		APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		FInputModeGameOnly InputModeData;

		PlayerController->SetInputMode(InputModeData);
		PlayerController->bShowMouseCursor = false;

		BuildingMenu->RemoveFromViewport();
		bWidgetOpen = false;

		if (Buildables.IsValidIndex(NewBuildId) && BuildId != NewBuildId)
		{
			BuildId = NewBuildId;
			UpdateMesh();
		}
	}
}


void UBuildManagerComponent::InteractWithBuild()
{
	if (!bWidgetOpen)
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
}


bool UBuildManagerComponent::DetectBuildBoxed(AActor* Actor, UPrimitiveComponent* Component)
{
	if (Actor->GetClass()->ImplementsInterface(UBuildInterface::StaticClass()))
	{
		int AssetId = IBuildInterface::Execute_GetAssetId(Actor);
		if (!Buildables.IsValidIndex(BuildId) || Buildables[BuildId].AssetId != AssetId)
		{
			return false;
		}

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


void UBuildManagerComponent::LoadBuildables(int TableId)
{
	if (Tables.IsValidIndex(TableId))
	{
		FTableInfo TableInfo = Tables[TableId];
		if (TableInfo.Group != Group || Buildables.Num() == 0)
		{
			UDataTable* Table = TableInfo.Table;
			TArray<FBuildable*> OutBuildables;

			Table->GetAllRows(FString("Buildables"), OutBuildables);
			Group = TableInfo.Group;
			BuildId = FMath::Clamp(BuildId, 0, OutBuildables.Num() - 1);
			Buildables.Empty();

			for (const auto& Buildable : OutBuildables)
			{
				Buildables.Emplace(*Buildable);
			}

			if (BuildGhost)
			{
				BuildingMenu->SetBuildId(BuildId);
			}
		}
	}
}


void UBuildManagerComponent::MoveBuildGhost(float Value)
{
	if (bIsBuildModeOn && BuildGhost)
	{
		BuildGhostOffset.Z = FMath::Clamp(BuildGhostOffset.Z + Value, -1000.0f, 1000.0f);
	}
}


void UBuildManagerComponent::RotateBuildGhost(float Value)
{
	if (bIsBuildModeOn && BuildGhost)
	{
		BuildGhostRotator.Yaw = int(BuildGhostRotator.Yaw + Value) % 360;
	}
}