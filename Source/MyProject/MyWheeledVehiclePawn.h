// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "MyWheeledVehiclePawn.generated.h"

/**
 * 
 */
UCLASS()
class MYPROJECT_API AMyWheeledVehiclePawn : public AWheeledVehiclePawn
{
	GENERATED_BODY()
	
	
	
	
	void SetAerofoilConfig();

	void SetThrusterConfig();


	void BeginPlay() override;
public:
	AMyWheeledVehiclePawn(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> AerofoilHealth;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> ThrusterHealth;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	float Health = 1000;;

	UPROPERTY(EditAnywhere)
	float DamageThreshold = 50000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageCeof = 500;

	UFUNCTION()
	void MeshHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION(BlueprintCallable)
	void Damaged(FName BoneName);

	UFUNCTION(BlueprintCallable)
	void DisableAerofoil(int Idx, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION(BlueprintCallable)
	void DisableThruster(int Idx, FVector NormalImpulse);

	UFUNCTION(BlueprintCallable)
	void ReceiveDamage(FName BoneName, float Damage, FVector NormalImpulse, const FHitResult& Hit);

	void Explode(const FName& BoneName, const FVector& NormalImpulse, bool& retFlag);

	UFUNCTION(BlueprintImplementableEvent)
	void AfterExplode();

	UFUNCTION(BlueprintImplementableEvent)
	void AfterDisabelThruster(FName BoneName);

	UFUNCTION(BlueprintCallable)
	void LaunchMissile();

	UFUNCTION(BlueprintCallable)
	void AddNewAerofoil(EVehicleAerofoilType AerofoilType, FName BoneName, FVector Offset, FVector UpAxis, float Area, float Camber, float MaxControlAngle, float StallAngle, float LiftMultiplier, float DragMultiplier);

	UFUNCTION(BlueprintCallable)
	void SetAerofoilArea(int Idx, float area);

	//void ReceivePointDamage(AActor* DamagedActor, float Damage, class AController* InstigatedBy, FVector HitLocation, class UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const class UDamageType* DamageType, AActor* DamageCauser) override;
};
