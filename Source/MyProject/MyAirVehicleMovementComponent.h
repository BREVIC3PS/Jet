// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "ChaosVehicleMovementComponent.h"
#include "MyAirVehicleMovementComponent.generated.h"


class MYPROJECT_API UMyChaosVehicleSimulation : public UChaosWheeledVehicleSimulation
{
public:
	virtual void ApplyAerofoilForces(float DeltaTime) override;

	void AddForceAtPosition(const FVector& Force, const FVector& Position, bool bAllowSubstepping = true, bool bIsLocalForce = false);

	bool addAreofoil;
};


/**
 * 
 */
UCLASS()
class MYPROJECT_API UMyAirVehicleMovementComponent : public UChaosWheeledVehicleMovementComponent
{
	GENERATED_BODY()
	
public:
	TArray<bool> AerofoilEnable;

	virtual void BeginPlay() override;

	virtual void OnCreatePhysicsState() override;
	virtual void OnDestroyPhysicsState() override;

	void RemoveAerofoilOrThruster(int32 AerofoilIndex, int32 ThrusterIndex);

	UFUNCTION(BlueprintCallable)
	void SetAerofoilArea(int AerofoilIdx, float NewArea);

	UFUNCTION(BlueprintCallable)
	float GetAerofoilArea(int AerofoilIdx);

	UFUNCTION(BlueprintCallable)
	void SetThrusterMaxForce(int ThrusterlIdx, float NewMaxForce);

	UFUNCTION()
	void AddNewAeroFoil(FVehicleAerofoilConfig NewFixedAerofoil);

	void OnDamaged();

	/** */
	virtual TUniquePtr<Chaos::FSimpleWheeledVehicle> CreatePhysicsVehicle() override
	{
		// Make the Vehicle Simulation class that will be updated from the physics thread async callback
		VehicleSimulationPT = MakeUnique<UMyChaosVehicleSimulation>();

		return UChaosVehicleMovementComponent::CreatePhysicsVehicle();
	}
	
};
