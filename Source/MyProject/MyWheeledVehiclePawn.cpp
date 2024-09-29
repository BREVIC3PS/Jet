// Fill out your copyright notice in the Description page of Project Settings.


#include "MyWheeledVehiclePawn.h"
#include "MyAirVehicleMovementComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Components/ActorComponent.h"
#include <Kismet/KismetMathLibrary.h>


void AMyWheeledVehiclePawn::BeginPlay()
{
	Super::BeginPlay();
	//OnTakePointDamage.AddDynamic(this, &AMyWheeledVehiclePawn::ReceivePointDamage);
	GetMesh()->OnComponentHit.AddDynamic(this, &AMyWheeledVehiclePawn::MeshHit);
	/*if (GetLocalRole() == ENetRole::ROLE_AutonomousProxy)
	{
		SetPhysicsReplicationMode(EPhysicsReplicationMode::Resimulation);
	}
	else if(GetLocalRole() == ENetRole::ROLE_SimulatedProxy)
	{
		SetPhysicsReplicationMode(EPhysicsReplicationMode::Default);
	}*/
}

AMyWheeledVehiclePawn::AMyWheeledVehiclePawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UMyAirVehicleMovementComponent>(TEXT("MyMovComponent")))
{
	SetAerofoilConfig();
	SetThrusterConfig();
}

void AMyWheeledVehiclePawn::MeshHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (NormalImpulse.Size() < DamageThreshold)return;
	float Damage = NormalImpulse.Size() / DamageCeof;
	ReceiveDamage(Hit.MyBoneName, Damage, NormalImpulse, Hit);
}

void AMyWheeledVehiclePawn::SetAerofoilConfig()
{
	FVehicleAerofoilConfig FixedAerofoilLeft;
	FixedAerofoilLeft.AerofoilType = EVehicleAerofoilType::Fixed;
	FixedAerofoilLeft.BoneName = FName("lf_slat_inner_jnt");
	FixedAerofoilLeft.Offset = FVector(0, 0, 0);
	FixedAerofoilLeft.UpAxis = FVector(0, 0, 1);
	FixedAerofoilLeft.Area = 4;
	FixedAerofoilLeft.Camber = 3;
	FixedAerofoilLeft.MaxControlAngle = 0;
	FixedAerofoilLeft.StallAngle = 16;
	FixedAerofoilLeft.LiftMultiplier = 1;
	FixedAerofoilLeft.DragMultiplier = 1;
	GetVehicleMovementComponent()->Aerofoils.Add(FixedAerofoilLeft);
	AerofoilHealth.Push(100);

	FVehicleAerofoilConfig FixedAerofoilRight;
	FixedAerofoilRight.AerofoilType = EVehicleAerofoilType::Fixed;
	FixedAerofoilRight.BoneName = FName("rt_slat_inner_jnt");
	FixedAerofoilRight.Offset = FVector(0, 0, 0);
	FixedAerofoilRight.UpAxis = FVector(0, 0, 1);
	FixedAerofoilRight.Area = 4;
	FixedAerofoilRight.Camber = 3;
	FixedAerofoilRight.MaxControlAngle = 0;
	FixedAerofoilRight.StallAngle = 16;
	FixedAerofoilRight.LiftMultiplier = 1;
	FixedAerofoilRight.DragMultiplier = 1;
	GetVehicleMovementComponent()->Aerofoils.Add(FixedAerofoilRight);
	AerofoilHealth.Push(100);

	FVehicleAerofoilConfig AerofoilWingLeft;
	AerofoilWingLeft.AerofoilType = EVehicleAerofoilType::Wing;
	AerofoilWingLeft.BoneName = FName("lf_aileron_outer_jnt");
	AerofoilWingLeft.Offset = FVector(0, 0, 0);
	AerofoilWingLeft.UpAxis = FVector(0, 0, 1);
	AerofoilWingLeft.Area = 3;
	AerofoilWingLeft.Camber = 3;
	AerofoilWingLeft.MaxControlAngle = 35;
	AerofoilWingLeft.StallAngle = 16;
	AerofoilWingLeft.LiftMultiplier = 1;
	AerofoilWingLeft.DragMultiplier = 1;
	GetVehicleMovementComponent()->Aerofoils.Add(AerofoilWingLeft);
	AerofoilHealth.Push(100);

	FVehicleAerofoilConfig AerofoilWingRight;
	AerofoilWingRight.AerofoilType = EVehicleAerofoilType::Wing;
	AerofoilWingRight.BoneName = FName("rt_aileron_outer_jnt");
	AerofoilWingRight.Offset = FVector(0, 0, 0);
	AerofoilWingRight.UpAxis = FVector(0, 0, 1);
	AerofoilWingRight.Area = 3;
	AerofoilWingRight.Camber = 3;
	AerofoilWingRight.MaxControlAngle = 35;
	AerofoilWingRight.StallAngle = 16;
	AerofoilWingRight.LiftMultiplier = 1;
	AerofoilWingRight.DragMultiplier = 1;
	GetVehicleMovementComponent()->Aerofoils.Add(AerofoilWingRight);
	AerofoilHealth.Push(100);

	FVehicleAerofoilConfig AerofoilRudderLeft;
	AerofoilRudderLeft.AerofoilType = EVehicleAerofoilType::Rudder;
	AerofoilRudderLeft.BoneName = FName("rt_rudder_jnt");
	AerofoilRudderLeft.Offset = FVector(0, 0, 0);
	AerofoilRudderLeft.UpAxis = FVector(0, 1, 0);
	AerofoilRudderLeft.Area = 2;
	AerofoilRudderLeft.Camber = 0;
	AerofoilRudderLeft.MaxControlAngle = 35;
	AerofoilRudderLeft.StallAngle = 16;
	AerofoilRudderLeft.LiftMultiplier = 1;
	AerofoilRudderLeft.DragMultiplier = 1;
	GetVehicleMovementComponent()->Aerofoils.Add(AerofoilRudderLeft);
	AerofoilHealth.Push(100);

	FVehicleAerofoilConfig AerofoilRudderRight;
	AerofoilRudderRight.AerofoilType = EVehicleAerofoilType::Rudder;
	AerofoilRudderRight.BoneName = FName("lf_rudder_jnt");
	AerofoilRudderRight.Offset = FVector(0, 0, 0);
	AerofoilRudderRight.UpAxis = FVector(0, 1, 0);
	AerofoilRudderRight.Area = 2;
	AerofoilRudderRight.Camber = 0;
	AerofoilRudderRight.MaxControlAngle = 35;
	AerofoilRudderRight.StallAngle = 16;
	AerofoilRudderRight.LiftMultiplier = 1;
	AerofoilRudderRight.DragMultiplier = 1;
	GetVehicleMovementComponent()->Aerofoils.Add(AerofoilRudderRight);
	AerofoilHealth.Push(100);

	FVehicleAerofoilConfig AerofoilElevatorLeft;
	AerofoilElevatorLeft.AerofoilType = EVehicleAerofoilType::Elevator;
	AerofoilElevatorLeft.BoneName = FName("lf_elevator_jnt");
	AerofoilElevatorLeft.Offset = FVector(0, 0, 0);
	AerofoilElevatorLeft.UpAxis = FVector(0, 0, 1);
	AerofoilElevatorLeft.Area = 3;
	AerofoilElevatorLeft.Camber = 0;
	AerofoilElevatorLeft.MaxControlAngle = 35;
	AerofoilElevatorLeft.StallAngle = 16;
	AerofoilElevatorLeft.LiftMultiplier = 1;
	AerofoilElevatorLeft.DragMultiplier = 1;
	GetVehicleMovementComponent()->Aerofoils.Add(AerofoilElevatorLeft);
	AerofoilHealth.Push(100);

	FVehicleAerofoilConfig AerofoilElevatorRight;
	AerofoilElevatorRight.AerofoilType = EVehicleAerofoilType::Elevator;
	AerofoilElevatorRight.BoneName = FName("rt_elevator_jnt");
	AerofoilElevatorRight.Offset = FVector(0, 0, 0);
	AerofoilElevatorRight.UpAxis = FVector(0, 0, 1);
	AerofoilElevatorRight.Area = 3;
	AerofoilElevatorRight.Camber = 0;
	AerofoilElevatorRight.MaxControlAngle = 35;
	AerofoilElevatorRight.StallAngle = 16;
	AerofoilElevatorRight.LiftMultiplier = 1;
	AerofoilElevatorRight.DragMultiplier = 1;
	GetVehicleMovementComponent()->Aerofoils.Add(AerofoilElevatorRight);
	AerofoilHealth.Push(100);
}

void AMyWheeledVehiclePawn::SetThrusterConfig()
{
	FVehicleThrustConfig ThrusterLeft;
	ThrusterLeft.BoneName = FName("lf_thruster_jnt");
	ThrusterLeft.ThrustAxis = FVector(1, 0, 0);
	ThrusterLeft.MaxThrustForce = 40000;
	GetVehicleMovementComponent()->Thrusters.Add(ThrusterLeft);
	ThrusterHealth.Push(100);

	FVehicleThrustConfig ThrusterRight;
	ThrusterLeft.BoneName = FName("rt_thruster_jnt");
	ThrusterLeft.ThrustAxis = FVector(1, 0, 0);
	ThrusterLeft.MaxThrustForce = 40000;
	GetVehicleMovementComponent()->Thrusters.Add(ThrusterLeft);
	ThrusterHealth.Push(100);
}

void AMyWheeledVehiclePawn::Damaged(FName BoneName)
{
	//DisableAerofoilOrThruster(BoneName, NormalImpulse);
	/*if (GetVehicleMovementComponent()->Aerofoils.IsEmpty() || GetVehicleMovementComponent()->Thrusters.IsEmpty())return;
	UMyAirVehicleMovementComponent* MyMovComp = Cast<UMyAirVehicleMovementComponent>(GetVehicleMovementComponent());
	if (!MyMovComp)return;
	for (int32 i=0;i< MyMovComp->Aerofoils.Num();i++)
	{
		if (MyMovComp->Aerofoils[i].BoneName == BoneName)
		{
			MyMovComp->RemoveAerofoilOrThruster(i, -1);
		}
	}

	for (int32 i = 0; i < MyMovComp->Thrusters.Num(); i++)
	{
		if (MyMovComp->Thrusters[i].BoneName == BoneName)
		{
			MyMovComp->RemoveAerofoilOrThruster(-1, i);
		}
	}*/
	
	/*GetVehicleMovementComponent()->ResetVehicleState();
	GetVehicleMovementComponent()->OnDestroyPhysicsState();
	GetVehicleMovementComponent()->OnCreatePhysicsState();*/
	
}

void AMyWheeledVehiclePawn::DisableAerofoil(int Idx, FVector NormalImpulse, const FHitResult& Hit)
{
	if (GetVehicleMovementComponent()->Aerofoils.IsEmpty() || GetVehicleMovementComponent()->Thrusters.IsEmpty())return;
	UMyAirVehicleMovementComponent* MyMovComp = Cast<UMyAirVehicleMovementComponent>(GetVehicleMovementComponent());
	if (!MyMovComp)return;
	//if(BoneName == FName)
	MyMovComp->SetAerofoilArea(Idx, 0);
	GetMesh()->BreakConstraint(NormalImpulse/100, Hit.ImpactPoint, MyMovComp->Aerofoils[Idx].BoneName);

}

void AMyWheeledVehiclePawn::DisableThruster(int Idx, FVector NormalImpulse)
{
	if (GetVehicleMovementComponent()->Aerofoils.IsEmpty() || GetVehicleMovementComponent()->Thrusters.IsEmpty())return;
	UMyAirVehicleMovementComponent* MyMovComp = Cast<UMyAirVehicleMovementComponent>(GetVehicleMovementComponent());
	if (!MyMovComp)return;

	MyMovComp->SetThrusterMaxForce(Idx, 0);
	AfterDisabelThruster(MyMovComp->Thrusters[Idx].BoneName);
}

void AMyWheeledVehiclePawn::ReceiveDamage(FName BoneName, float Damage, FVector NormalImpulse, const FHitResult& Hit)
{
	if (BoneName == FName("cog_jnt"))
	{
		Health -= Damage;
		if (Health < 0)
		{
			bool retFlag;
			Explode(BoneName, NormalImpulse, retFlag);
			
		}
		return;
	}

	if (GetVehicleMovementComponent()->Aerofoils.IsEmpty() || GetVehicleMovementComponent()->Thrusters.IsEmpty())
		return;

	UMyAirVehicleMovementComponent* MyMovComp = Cast<UMyAirVehicleMovementComponent>(GetVehicleMovementComponent());
	if (!MyMovComp)
		return;

	for (int32 i = 0; i < MyMovComp->Aerofoils.Num(); i++)
	{
		if (MyMovComp->Aerofoils[i].BoneName == BoneName)
		{
			AerofoilHealth[i] -= Damage;
			if (AerofoilHealth[i] <= 0)
			{
				DisableAerofoil(i, NormalImpulse, Hit);
			}
			else
			{
				float CurrentArea = MyMovComp->GetAerofoilArea(i);
				MyMovComp->SetAerofoilArea(i, (AerofoilHealth[i] / 100) * MyMovComp->Aerofoils[i].Area);
			}
			return;
		}
	}

	for (int32 i = 0; i < MyMovComp->Thrusters.Num(); i++)
	{
		if (MyMovComp->Thrusters[i].BoneName == BoneName)
		{
			ThrusterHealth[i] -= Damage;
			if (ThrusterHealth[i] - Damage <= 0)
			{
				DisableThruster(i, NormalImpulse);
			}
			else
			{
				MyMovComp->SetThrusterMaxForce(i, (ThrusterHealth[i] / 100) * MyMovComp->Thrusters[i].MaxThrustForce);
			}
		}
	}

	
}

void AMyWheeledVehiclePawn::Explode(const FName& BoneName, const FVector& NormalImpulse, bool& retFlag)
{
	retFlag = true;
	if (GetVehicleMovementComponent()->Aerofoils.IsEmpty() || GetVehicleMovementComponent()->Thrusters.IsEmpty())return;
	UMyAirVehicleMovementComponent* MyMovComp = Cast<UMyAirVehicleMovementComponent>(GetVehicleMovementComponent());
	if (!MyMovComp)return;
	for (int32 i = 0; i < MyMovComp->Aerofoils.Num(); i++)
	{
		FHitResult Hit;
		Hit.ImpactPoint = GetActorLocation();
		DisableAerofoil(i, NormalImpulse, Hit);
	}
	for (int32 i = 0; i < MyMovComp->Thrusters.Num(); i++)
	{
		DisableThruster(i, NormalImpulse);
	}
	AfterExplode();
	retFlag = false;
}

void AMyWheeledVehiclePawn::LaunchMissile()
{

}

void AMyWheeledVehiclePawn::AddNewAerofoil(EVehicleAerofoilType AerofoilType, FName BoneName, FVector Offset, FVector UpAxis, float Area, float Camber, float MaxControlAngle, float StallAngle, float LiftMultiplier, float DragMultiplier)
{
	FVehicleAerofoilConfig NewFixedAerofoil;
	NewFixedAerofoil.AerofoilType = AerofoilType;
	NewFixedAerofoil.BoneName = BoneName;
	NewFixedAerofoil.Offset = Offset;
	NewFixedAerofoil.UpAxis = UpAxis;
	NewFixedAerofoil.Camber = Camber;
	NewFixedAerofoil.MaxControlAngle = MaxControlAngle;
	NewFixedAerofoil.StallAngle = StallAngle;
	NewFixedAerofoil.LiftMultiplier = LiftMultiplier;
	NewFixedAerofoil.DragMultiplier = DragMultiplier;

	UMyAirVehicleMovementComponent* MyMovComp = Cast<UMyAirVehicleMovementComponent>(GetVehicleMovementComponent());
	if (MyMovComp)
	{
		MyMovComp->AddNewAeroFoil(NewFixedAerofoil);
	}
}

void AMyWheeledVehiclePawn::SetAerofoilArea(int Idx, float area)
{
	UMyAirVehicleMovementComponent* MyMovComp = Cast<UMyAirVehicleMovementComponent>(GetVehicleMovementComponent());
	if (MyMovComp)
	{
		MyMovComp->SetAerofoilArea(Idx, area);
	}
}

//void AMyWheeledVehiclePawn::ReceivePointDamage(AActor* DamagedActor, float Damage, AController* InstigatedBy, FVector HitLocation, UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const UDamageType* DamageType, AActor* DamageCauser)
//{
//	
//}

