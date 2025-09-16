#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "VehiclePawn.generated.h"

UCLASS()
class STEERWARS_API AVehiclePawn : public APawn
{
	GENERATED_BODY()

public:
	AVehiclePawn();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Functions
	void SuspensionCast(USceneComponent* WheelComp);
	void AccelerateVehicle(float DeltaTime);
	void CalculateAcceleration(float DeltaTime);
	void OnAccelerate(const FInputActionValue& Value);

public:
	// Components
	UPROPERTY(VisibleAnywhere)
	class UBoxComponent* CollisionBox;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Chassis;
	
	UPROPERTY(Category = Camera, EditDefaultsOnly)
	class USpringArmComponent* SpringArm;
	
	UPROPERTY(Category = Camera, EditDefaultsOnly)
	class UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere)
	TArray<USceneComponent*> WheelPoints;

	// Input
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	class UInputMappingContext* VehicleMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	class UInputAction* AccelerateAction;

	// Vehicle Movement Variables
	UPROPERTY(EditAnywhere, Category = "Vehicle|Movement")
	float MaxAcceleration = 15000.0f;

	UPROPERTY(EditAnywhere, Category = "Vehicle|Movement")
	float AccelInterpSpeed = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Vehicle|Movement")
	float DecelInterpSpeed = 0.3f;

	UPROPERTY(EditAnywhere, Category = "Vehicle|Movement")
	float ForceScale = 1000.0f;

private:
	// Input state
	float AccelerationInput = 0.0f; // Current smoothed input (-1 to +1)
	float TargetInput = 0.0f; // Where input wants to go

	// Calculated acceleration force
	float CurrentAcceleration = 0.0f;
};
