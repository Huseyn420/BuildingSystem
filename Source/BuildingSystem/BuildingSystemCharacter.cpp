// Copyright Epic Games, Inc. All Rights Reserved.

#include "BuildingSystemCharacter.h"
#include "BuildInterface.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "BuildManagerComponent.h"
#include "ProgressWidget.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameUserSettings.h"


ABuildingSystemCharacter::ABuildingSystemCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bInProgress = false;
	ProgressValue = 0.0f;

	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	MainCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("MainCamera"));
	MainCamera->SetupAttachment(GetMesh(), "main_camera");
	MainCamera->bUsePawnControlRotation = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	BuildManager = CreateDefaultSubobject<UBuildManagerComponent>(TEXT("BuildManager"));
	BuildManager->Scene = RootComponent;
	BuildManager->Camera = FollowCamera;

	bUseControllerRotationYaw = true;
}


void ABuildingSystemCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocallyControlled())
	{
		APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		ProgressWidget = CreateWidget<UProgressWidget>(PlayerController, UProgressWidget::StaticClass());
		ProgressWidget->AddToViewport();
	}

	UGameUserSettings* pGameUserSettings = UGameUserSettings::GetGameUserSettings();
	if (pGameUserSettings)
	{
		pGameUserSettings->SetFullscreenMode(EWindowMode::Fullscreen);
		pGameUserSettings->ApplySettings(false);	
	}
}


void ABuildingSystemCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &ABuildingSystemCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ABuildingSystemCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &ABuildingSystemCharacter::Turn);
	PlayerInputComponent->BindAxis("TurnRate", this, &ABuildingSystemCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ABuildingSystemCharacter::LookUpAtRate);

	PlayerInputComponent->BindTouch(IE_Pressed, this, &ABuildingSystemCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ABuildingSystemCharacter::TouchStopped);

	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &ABuildingSystemCharacter::OnResetVR);

	PlayerInputComponent->BindAxis("UpdateBuildMesh", this, &ABuildingSystemCharacter::UpdateBuildMesh);
	PlayerInputComponent->BindAxis("RotateBuildGhost", this, &ABuildingSystemCharacter::RotateBuildGhost);

	PlayerInputComponent->BindAction("RemoveBuilding", IE_Pressed, this, &ABuildingSystemCharacter::RemoveBuilding);
	PlayerInputComponent->BindAction("RemoveBuilding", IE_Released, this, &ABuildingSystemCharacter::StopDeleteBuilding);

	PlayerInputComponent->BindAction("LaunchBuildMode", IE_Pressed, BuildManager, &UBuildManagerComponent::LaunchBuildMode);
	PlayerInputComponent->BindAction("DealDamage", IE_Pressed, BuildManager, &UBuildManagerComponent::DealDamage);
	PlayerInputComponent->BindAction("SpawnBuild", IE_Pressed, BuildManager, &UBuildManagerComponent::SpawnBuild);
	PlayerInputComponent->BindAction("InteractWithBuild", IE_Pressed, BuildManager, &UBuildManagerComponent::InteractWithBuild);

	PlayerInputComponent->BindAction("ChangeCamera", IE_Pressed, this, &ABuildingSystemCharacter::ChangeCamera);
	PlayerInputComponent->BindAction("QuitGame", IE_Pressed, this, &ABuildingSystemCharacter::QuitGame);
}


void ABuildingSystemCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void ABuildingSystemCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void ABuildingSystemCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}


void ABuildingSystemCharacter::Turn(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());

	if (MainCamera->IsActive() && (Controller != nullptr) && (Rate != 0.0f))
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		FaceRotation(YawRotation, GetWorld()->GetDeltaSeconds());
		RotateBuildGhost(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
	}
}


void ABuildingSystemCharacter::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ABuildingSystemCharacter::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ABuildingSystemCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ABuildingSystemCharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
}


void ABuildingSystemCharacter::UpdateBuildMesh(float Value)
{
	if (Value != 0)
	{
		int NewValue = round(Value) + BuildManager->BuildId;
		BuildManager->BuildId = FMath::Clamp(NewValue, 0, BuildManager->Buildables.Num() - 1);
		BuildManager->UpdateMesh();
	}
}


void ABuildingSystemCharacter::RemoveBuilding()
{
	if (bInProgress == false)
	{
		FHitResult TraceResult;
		UCameraComponent* Camera = FollowCamera->IsActive() ? FollowCamera : MainCamera;
		FVector LineStart = GetActorLocation() + Camera->GetForwardVector();
		FVector LineEnd = GetActorLocation() + Camera->GetForwardVector() * 2000.0f;

		if (GetWorld()->LineTraceSingleByChannel(TraceResult, LineStart, LineEnd, ECC_Visibility))
		{
			ActorBeingDeleted = TraceResult.GetActor();
			if (ActorBeingDeleted->GetClass()->ImplementsInterface(UBuildInterface::StaticClass()))
			{
				bInProgress = true;
				ProgressWidget->SetVisibility(ESlateVisibility::Visible);

				GetWorld()->GetTimerManager().SetTimer(TimerHandle, [&]()
				{
					if (ProgressValue < 1.0f)
					{
						ProgressWidget->SetProgress(ProgressValue);
						ProgressValue += 0.01f;
					}
					else
					{
						ActorBeingDeleted->Destroy();
						StopDeleteBuilding();
					}
				}, 0.01, true);
			}
		}
	}
}


void ABuildingSystemCharacter::StopDeleteBuilding()
{
	ProgressValue = 0.0f;
	ActorBeingDeleted = nullptr;
	bInProgress = false;

	ProgressWidget->SetProgress(ProgressValue);
	ProgressWidget->SetVisibility(ESlateVisibility::Collapsed);

	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
}


void ABuildingSystemCharacter::RotateBuildGhost(float Value)
{
	if (Value != 0)
	{
		BuildManager->RotateBuildGhost(Value);
	}
}


void ABuildingSystemCharacter::ChangeCamera()
{
	if (MainCamera->IsActive())
	{
		FollowCamera->Activate();
		MainCamera->Deactivate();
		BuildManager->Camera = FollowCamera;
		bUseControllerRotationYaw = false;
	}
	else
	{
		MainCamera->Activate();
		FollowCamera->Deactivate();
		BuildManager->Camera = MainCamera;
		bUseControllerRotationYaw = true;
	}
}


void ABuildingSystemCharacter::QuitGame()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), Cast<APlayerController>(GetController()), EQuitPreference::Quit, false);
}