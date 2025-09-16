#include "SteerWars/Car/VehiclePawn.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"

AVehiclePawn::AVehiclePawn()
{
	PrimaryActorTick.bCanEverTick = true;

	// Collision Box (root)
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	RootComponent = CollisionBox;
	CollisionBox->SetSimulatePhysics(true);
	CollisionBox->SetCollisionProfileName(TEXT("BlockAll"));

	// Chassis Mesh
	Chassis = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Chassis"));
	Chassis->SetupAttachment(RootComponent);

	// Spring Arm
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 500.0f;

	// Camera
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	// Create 4 placeholder wheel scene components
	for (int32 i = 0; i < 4; i++)
	{
		USceneComponent* Wheel = CreateDefaultSubobject<USceneComponent>(*FString::Printf(TEXT("Wheel_%d"), i));
		Wheel->SetupAttachment(RootComponent);
		WheelPoints.Add(Wheel);
	}
}

void AVehiclePawn::BeginPlay()
{
	Super::BeginPlay();

	// Register Input Mapping Context at runtime
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(VehicleMappingContext, 0);
		}
	}
}

void AVehiclePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Suspension
	for (USceneComponent* Wheel : WheelPoints)
	{
		SuspensionCast(Wheel);
	}

	// Handle acceleration smoothing and force
	CalculateAcceleration(DeltaTime);
	AccelerateVehicle(DeltaTime);
}

void AVehiclePawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInput->BindAction(AccelerateAction, ETriggerEvent::Triggered, this, &AVehiclePawn::OnAccelerate);
		EnhancedInput->BindAction(AccelerateAction, ETriggerEvent::Completed, this, &AVehiclePawn::OnAccelerate);
	}
}

void AVehiclePawn::AccelerateVehicle(float DeltaTime)
{
	if (!CollisionBox) return;

	for (USceneComponent* Wheel : WheelPoints)
	{
		if (!Wheel) continue;

		FVector Forward = Chassis->GetForwardVector();
		float Mass = CollisionBox->GetMass();

		// Force = Forward * Acceleration * ForceScale * Mass
		FVector Force = Forward * CurrentAcceleration * ForceScale * Mass * DeltaTime;

		CollisionBox->AddForceAtLocation(Force, Wheel->GetComponentLocation());

		// Shift center of mass slightly when accelerating
		FVector COMOffset(50.f, 0.f, AccelerationInput * -20.f);
		CollisionBox->SetCenterOfMass(COMOffset);
	}
}

void AVehiclePawn::CalculateAcceleration(float DeltaTime)
{
	// Smooth input interpolation
	AccelerationInput = FMath::FInterpTo(AccelerationInput, TargetInput, DeltaTime,
		(TargetInput == 0.0f ? DecelInterpSpeed : AccelInterpSpeed));

	// Acceleration force calculation (lerp 0 -> MaxAcceleration)
	CurrentAcceleration = FMath::Lerp(0.0f,MaxAcceleration, FMath::Abs(AccelerationInput));

	// Apply sign (forward/backward)
	CurrentAcceleration *= AccelerationInput;
}

void AVehiclePawn::OnAccelerate(const FInputActionValue& Value)
{
	// W gives +1, S gives -1
	TargetInput = Value.Get<float>();
}

void AVehiclePawn::SuspensionCast(USceneComponent* WheelComp)
{
	if (!WheelComp || !CollisionBox) return;

	FVector Start = WheelComp->GetComponentLocation();
	FVector End = Start - FVector(0, 0, 60);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

	DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 0.05f, 0, 2.0f);

	if (bHit && Hit.bBlockingHit)
	{
		float Distance = Hit.Distance;

		// Normalise distance to 0-1 range and invert
		float Normalised = FMath::Clamp((1.0f - (Distance / 60.0f)), 0.0f, 1.0f);

		FVector Force = FVector(0, 0, 1) * Normalised * 90000.0f;

		CollisionBox->AddForceAtLocation(Force, WheelComp->GetComponentLocation());
	}
}
