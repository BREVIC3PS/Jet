// Fill out your copyright notice in the Description page of Project Settings.


#include "MyAirVehicleMovementComponent.h"
#include "ChaosVehicleManager.h"
#include "PhysicsProxy/SingleParticlePhysicsProxy.h"
#include <Chaos/DebugDrawQueue.h>
#include "AerofoilSystem.h"

FVehicleDebugParams GVehicleDebugParams;

FAutoConsoleVariableRef CVarChaosVehiclesShowCOM(TEXT("p.Vehicle.ShowCOM"), GVehicleDebugParams.ShowCOM, TEXT("Enable/Disable Center Of Mass Debug Visualisation."));
FAutoConsoleVariableRef CVarChaosVehiclesShowModelAxis(TEXT("p.Vehicle.ShowModelOrigin"), GVehicleDebugParams.ShowModelOrigin, TEXT("Enable/Disable Model Origin Visualisation."));
FAutoConsoleVariableRef CVarChaosVehiclesShowAllForces(TEXT("p.Vehicle.ShowAllForces"), GVehicleDebugParams.ShowAllForces, TEXT("Enable/Disable Force Visualisation."));
FAutoConsoleVariableRef CVarChaosVehiclesAerofoilForces(TEXT("p.Vehicle.ShowAerofoilForces"), GVehicleDebugParams.ShowAerofoilForces, TEXT("Enable/Disable Aerofoil Force Visualisation."));
FAutoConsoleVariableRef CVarChaosVehiclesAerofoilSurface(TEXT("p.Vehicle.ShowAerofoilSurface"), GVehicleDebugParams.ShowAerofoilSurface, TEXT("Enable/Disable a very approximate visualisation of where the Aerofoil surface is located and its orientation."));
FAutoConsoleVariableRef CVarChaosVehiclesDisableTorqueControl(TEXT("p.Vehicle.DisableTorqueControl"), GVehicleDebugParams.DisableTorqueControl, TEXT("Enable/Disable Direct Torque Control."));
FAutoConsoleVariableRef CVarChaosVehiclesDisableStabilizeControl(TEXT("p.Vehicle.DisableStabilizeControl"), GVehicleDebugParams.DisableStabilizeControl, TEXT("Enable/Disable Position Stabilization Control."));
FAutoConsoleVariableRef CVarChaosVehiclesDisableAerodynamics(TEXT("p.Vehicle.DisableAerodynamics"), GVehicleDebugParams.DisableAerodynamics, TEXT("Enable/Disable Aerodynamic Forces Drag/Downforce."));
FAutoConsoleVariableRef CVarChaosVehiclesDisableAerofoils(TEXT("p.Vehicle.DisableAerofoils"), GVehicleDebugParams.DisableAerofoils, TEXT("Enable/Disable Aerofoil Forces."));
FAutoConsoleVariableRef CVarChaosVehiclesDisableThrusters(TEXT("p.Vehicle.DisableThrusters"), GVehicleDebugParams.DisableThrusters, TEXT("Enable/Disable Thruster Forces."));
FAutoConsoleVariableRef CVarChaosVehiclesBatchQueries(TEXT("p.Vehicle.BatchQueries"), GVehicleDebugParams.BatchQueries, TEXT("Enable/Disable Batching Of Suspension Raycasts."));
FAutoConsoleVariableRef CVarChaosVehiclesCacheTraceOverlap(TEXT("p.Vehicle.CacheTraceOverlap"), GVehicleDebugParams.CacheTraceOverlap, TEXT("Enable/Disable Caching Of Suspension Trace Overlap Test Optimization (only valid when BatchQueries enabled)."));
FAutoConsoleVariableRef CVarChaosVehiclesForceDebugScaling(TEXT("p.Vehicle.SetForceDebugScaling"), GVehicleDebugParams.ForceDebugScaling, TEXT("Set Scaling For Force Visualisation."));
FAutoConsoleVariableRef CVarChaosVehiclesSleepCounterThreshold(TEXT("p.Vehicle.SleepCounterThreshold"), GVehicleDebugParams.SleepCounterThreshold, TEXT("Set The Sleep Counter Iteration Threshold."));
FAutoConsoleVariableRef CVarChaosVehiclesDisableVehicleSleep(TEXT("p.Vehicle.DisableVehicleSleep"), GVehicleDebugParams.DisableVehicleSleep, TEXT("Disable Vehicle Agressive Sleeping."));
FAutoConsoleVariableRef CVarChaosVehiclesSetMaxMPH(TEXT("p.Vehicle.SetMaxMPH"), GVehicleDebugParams.SetMaxMPH, TEXT("Set a top speed in MPH (affects all vehicles)."));
FAutoConsoleVariableRef CVarChaosVehiclesEnableMultithreading(TEXT("p.Vehicle.EnableMultithreading"), GVehicleDebugParams.EnableMultithreading, TEXT("Enable multi-threading of vehicle updates."));
FAutoConsoleVariableRef CVarChaosVehiclesControlInputWakeTolerance(TEXT("p.Vehicle.ControlInputWakeTolerance"), GVehicleDebugParams.ControlInputWakeTolerance, TEXT("Set the control input wake tolerance."));

FVector GetWorldVelocityAtPoint(const Chaos::FRigidBodyHandle_Internal* RigidHandle, const FVector& WorldLocation)
{
	if (RigidHandle)
	{
		const Chaos::FVec3 COM = RigidHandle ? Chaos::FParticleUtilitiesGT::GetCoMWorldPosition(RigidHandle) : (Chaos::FVec3)Chaos::FParticleUtilitiesGT::GetActorWorldTransform(RigidHandle).GetTranslation();
		const Chaos::FVec3 Diff = WorldLocation - COM;
		return RigidHandle->V() - Chaos::FVec3::CrossProduct(Diff, RigidHandle->W());
	}
	else
	{
		return FVector::ZeroVector;
	}
}

const double SeaLevelTemp = 288.15; // 海平面温度 (K)
const double SeaLevelPressure = 101325; // 海平面压力 (Pa)
const double TempLapseRate = 0.0065; // 温度梯度 (K/m)
const double Gravity = 9.80665; // 重力加速度 (m/s?)
const double GasConstant = 287.05; // 空气的比气体常数 (J/(kg·K))

double CalculateAirDensity(double altitude)
{
	// 计算高度处的温度
	double tempAtAltitude = SeaLevelTemp - TempLapseRate * altitude;

	// 计算高度处的压力
	double pressureAtAltitude = SeaLevelPressure * pow((tempAtAltitude / SeaLevelTemp), (Gravity / (TempLapseRate * GasConstant)));

	// 计算高度处的空气密度
	double airDensity = pressureAtAltitude / (GasConstant * tempAtAltitude);

	return airDensity;
}

void UMyChaosVehicleSimulation::ApplyAerofoilForces(float DeltaTime)
{
	using namespace Chaos;
	if (GVehicleDebugParams.DisableAerofoils || RigidHandle == nullptr)
		return;

	TArray<FVector> VelocityLocal;
	TArray<FVector> VelocityWorld;
	VelocityLocal.SetNum(PVehicle->Aerofoils.Num());
	VelocityWorld.SetNum(PVehicle->Aerofoils.Num());

	float Altitude = VehicleState.VehicleWorldTransform.GetLocation().Z;

	// Work out velocity at each aerofoil before applying any forces so there's no bias on the first ones processed
	for (int AerofoilIdx = 0; AerofoilIdx < PVehicle->Aerofoils.Num(); AerofoilIdx++)
	{
		FVector WorldLocation = VehicleState.VehicleWorldTransform.TransformPosition(PVehicle->GetAerofoil(AerofoilIdx).Setup().Offset);
		VelocityWorld[AerofoilIdx] = GetWorldVelocityAtPoint(RigidHandle, WorldLocation);
		VelocityLocal[AerofoilIdx] = VehicleState.VehicleWorldTransform.InverseTransformVector(VelocityWorld[AerofoilIdx]);
	}

	for (int AerofoilIdx = 0; AerofoilIdx < PVehicle->Aerofoils.Num(); AerofoilIdx++)
	{
		Chaos::FAerofoil& Aerofoil = PVehicle->GetAerofoil(AerofoilIdx);
		Aerofoil.SetDensityOfMedium(CalculateAirDensity(Chaos::CmToM(Altitude)));

		FVector LocalForce = Aerofoil.GetForce(VehicleState.VehicleWorldTransform, VelocityLocal[AerofoilIdx] * Chaos::CmToMScaling(), Chaos::CmToM(Altitude), DeltaTime);

		FVector WorldForce = VehicleState.VehicleWorldTransform.TransformVector(LocalForce);
		FVector WorldLocation = VehicleState.VehicleWorldTransform.TransformPosition(Aerofoil.GetCenterOfLiftOffset());
		AddForceAtPosition(WorldForce * Chaos::MToCmScaling(), WorldLocation);

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
		FVector WorldAxis = VehicleState.VehicleWorldTransform.TransformVector(FVector::CrossProduct(FVector(1, 0, 0), Aerofoil.Setup().UpAxis));
			Chaos::FDebugDrawQueue::GetInstance().DrawDebugLine(WorldLocation - WorldAxis * 150.0f, WorldLocation + WorldAxis * 150.0f, FColor::Black, false, -1.f, 0, 5.f);
			Chaos::FDebugDrawQueue::GetInstance().DrawDebugLine(WorldLocation, WorldLocation + WorldForce * GVehicleDebugParams.ForceDebugScaling, FColor::Green, false, -1.f, 0, 16.f);
			Chaos::FDebugDrawQueue::GetInstance().DrawDebugDirectionalArrow(WorldLocation, WorldLocation - VelocityWorld[AerofoilIdx] * Chaos::MToCmScaling() * GVehicleDebugParams.ForceDebugScaling
				, 20.f, FColor::White, false, 0, 0, 2.f);
#endif
	}

		/*if (GVehicleDebugParams.ShowAerofoilSurface)
		{
		}
		if (GVehicleDebugParams.ShowAerofoilForces)
		{
		}*/
}

void UMyChaosVehicleSimulation::AddForceAtPosition(const FVector& Force, const FVector& Position, bool bAllowSubstepping, bool bIsLocalForce)
{
	DeferredForces.Add(FDeferredForces::FApplyForceAtPositionData(Force, Position, bAllowSubstepping, bIsLocalForce));

		Chaos::FDebugDrawQueue::GetInstance().DrawDebugDirectionalArrow(Position, Position + Force * GVehicleDebugParams.ForceDebugScaling
			, 20.f, FColor::Orange, false, 0, 0, 2.f);
}

void UMyAirVehicleMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	for (int AerofoilIdx = 0; AerofoilIdx < Aerofoils.Num(); AerofoilIdx++)
	{
		AerofoilEnable.Push(true);
	}
}

void UMyAirVehicleMovementComponent::OnCreatePhysicsState()
{
	SetAsyncPhysicsTickEnabled(true);
	Super::OnCreatePhysicsState();
}

void UMyAirVehicleMovementComponent::OnDestroyPhysicsState()
{
	Super::OnDestroyPhysicsState();
}

void UMyAirVehicleMovementComponent::RemoveAerofoilOrThruster(int32 AerofoilIndex, int32 ThrusterIndex)
{
	if (AerofoilIndex != -1 && Aerofoils.Num() > AerofoilIndex)
	{
		Aerofoils.RemoveAt(AerofoilIndex);
	}
	if (ThrusterIndex != -1 && Thrusters.Num() > ThrusterIndex)
	{
		Thrusters.RemoveAt(ThrusterIndex);
	}
	OnDamaged();
}

void UMyAirVehicleMovementComponent::SetAerofoilArea(int AerofoilIdx, float NewArea)
{
	if (FBodyInstance* TargetInstance = GetBodyInstance())
	{
		FPhysicsCommand::ExecuteWrite(TargetInstance->ActorHandle, [&](const FPhysicsActorHandle& Chassis)
			{
				if (VehicleSimulationPT && VehicleSimulationPT->PVehicle && AerofoilIdx < VehicleSimulationPT->PVehicle->Aerofoils.Num())
				{
					VehicleSimulationPT->PVehicle->GetAerofoil(AerofoilIdx).AccessSetup().Area = NewArea;
				}
			});
	}

}

float UMyAirVehicleMovementComponent::GetAerofoilArea(int AerofoilIdx)
{
	float ret =0;
	if (FBodyInstance* TargetInstance = GetBodyInstance())
	{
		FPhysicsCommand::ExecuteWrite(TargetInstance->ActorHandle, [&](const FPhysicsActorHandle& Chassis)
			{
				if (VehicleSimulationPT && VehicleSimulationPT->PVehicle && AerofoilIdx < VehicleSimulationPT->PVehicle->Aerofoils.Num())
				{
					ret = VehicleSimulationPT->PVehicle->GetAerofoil(AerofoilIdx).Setup().Area;
				}

			});
	}
	return ret;
}

void UMyAirVehicleMovementComponent::SetThrusterMaxForce(int ThrusterlIdx, float NewMaxForce)
{
	if (FBodyInstance* TargetInstance = GetBodyInstance())
	{
		FPhysicsCommand::ExecuteWrite(TargetInstance->ActorHandle, [&](const FPhysicsActorHandle& Chassis)
			{
				if (VehicleSimulationPT && VehicleSimulationPT->PVehicle && ThrusterlIdx < VehicleSimulationPT->PVehicle->Thrusters.Num())
				{
					Chaos::FSimpleThrustSim& Thruster = VehicleSimulationPT->PVehicle->Thrusters[ThrusterlIdx];

					Thruster.AccessSetup().MaxThrustForce = NewMaxForce;
				}
			});
	}
}

void UMyAirVehicleMovementComponent::AddNewAeroFoil(FVehicleAerofoilConfig NewFixedAerofoil)
{

	Aerofoils.Add(NewFixedAerofoil);

	Chaos::FAerofoil AerofoilSim(&NewFixedAerofoil.GetPhysicsAerofoilConfig(*this));
	if (FBodyInstance* TargetInstance = GetBodyInstance())
	{
		FPhysicsCommand::ExecuteWrite(TargetInstance->ActorHandle, [&](const FPhysicsActorHandle& Chassis)
			{
				if (VehicleSimulationPT && VehicleSimulationPT->PVehicle)
				{
					//VehicleSimulationPT->PVehicle->Aerofoils.Add(AerofoilSim);
					//VehicleSimulationPT->addAreofoil = true;
				}
			});
	}
}


//deprecated
void UMyAirVehicleMovementComponent::OnDamaged()
{
	VehicleSetupTag = FChaosVehicleManager::VehicleSetupTag;

	// only create Physics vehicle in game
	UWorld* World = GetWorld();
	if (World->IsGameWorld())
	{
		if (FPhysScene* PhysScene = World->GetPhysicsScene())
		{
			if (FChaosVehicleManager::GetVehicleManagerFromScene(PhysScene))
			{
				CreateVehicle();
				FixupSkeletalMesh();

				/*if (PVehicleOutput)
				{
 						FChaosVehicleManager* VehicleManager = FChaosVehicleManager::GetVehicleManagerFromScene(PhysScene);
						VehicleManager->AddVehicle(this);
				}*/
				
				
				if (bUsingNetworkPhysicsPrediction)
				{
					if (NetworkPhysicsComponent)
					{
						NetworkPhysicsComponent->CreateDataHistory<FPhysicsVehicleTraits>(this);
					}
				}
			}
		}
	}

	FBodyInstance* BodyInstance = nullptr;
	if (USkeletalMeshComponent* SkeletalMesh = GetSkeletalMesh())
	{
		// this line was causing the server wheel positions to not be updated - this is already a user property so don't override it here and leave it to user to select the right option for their scenario
		//SkeletalMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
		BodyInstance = &SkeletalMesh->BodyInstance;
	}
}


