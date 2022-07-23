// Fill out your copyright notice in the Description page of Project Settings.

#include "JoyBlueprintFunctionLibrary.h"

#include <CoreGlobals.h>
#include <Kismet/GameplayStatics.h>
#include <Components/WidgetComponent.h>
#include <Components/SkeletalMeshComponent.h>
#include <Components/ShapeComponent.h>
#include <Components/StaticMeshComponent.h>
#include <Animation/AnimSingleNodeInstance.h>
#include <Navigation/PathFollowingComponent.h>
#include <NavigationPath.h>
#include <AIController.h>
#include <Engine/LevelStreaming.h>
#if WITH_EDITOR
#include <Editor.h>
#endif

#include <Assert.h>

#include "Assert.h"
#include "DebugTools.h"
#include "JoyGameInstance.h"
#include "Log.h"
#include "MacroUtils.h"
#include "StringTools.h"


int UJoyBlueprintFunctionLibrary::GetCurrentFrameNumber()
{
	return GFrameNumber;
}

void UJoyBlueprintFunctionLibrary::SetWidgetComponentWidgetClass(UWidgetComponent* _component, TSubclassOf<UUserWidget> _class)
{
	_component->SetWidgetClass(_class);
}

bool UJoyBlueprintFunctionLibrary::IsWithEditorBuild()
{
#if WITH_EDITOR
	return true;
#else
	return false;
#endif
}

bool UJoyBlueprintFunctionLibrary::IsAssertEnabled()
{
#if JOY_ASSERT_ENABLED
	return true;
#else
	return false;
#endif
}

bool UJoyBlueprintFunctionLibrary::IsPlayInEditor(UObject* _worldContextObject)
{
	if (_worldContextObject)
	{
		return _worldContextObject->GetWorld()->IsPlayInEditor();
	}
	
	return false;
}

bool UJoyBlueprintFunctionLibrary::IsSimulatingInEditor()
{
#if WITH_EDITOR
	if (GEditor)
	{
		return GEditor->bIsSimulatingInEditor || GEditor->bIsSimulateInEditorQueued;
	}
#endif
	return false;
}

void UJoyBlueprintFunctionLibrary::Assert(bool _condition, FString _message)
{
	JOY_ASSERT_MSGF(_condition, TEXT("%s"), *_message);
}

int UJoyBlueprintFunctionLibrary::HashString(const FString& _string)
{
	return ::HashString(_string);
}

void UJoyBlueprintFunctionLibrary::SetShapeNavigationRelevancy(UShapeComponent* _shapeComponent, bool _relevancy)
{
	JOY_EXITCONDITION(!_shapeComponent, TEXT("Undefined ShapeComponent."));

	_shapeComponent->SetCanEverAffectNavigation(_relevancy);
}

bool UJoyBlueprintFunctionLibrary::GetShapeNavigationRelevancy(UShapeComponent* _shapeComponent)
{
	JOY_EXITCONDITION_RET(!_shapeComponent, false, TEXT("Undefined ShapeComponent."));

	return _shapeComponent->CanEverAffectNavigation();
}

void UJoyBlueprintFunctionLibrary::GetPathComponentCurrentPath(const UPathFollowingComponent* _pathFollowingComponent, UNavigationPath* _outPath)
{
	JOY_EXITCONDITION(!_pathFollowingComponent, TEXT("Undefined PathFollowingComponent."));

	const FNavPathSharedPtr Path = _pathFollowingComponent->GetPath();
	_outPath->SetPath(Path);
}

float UJoyBlueprintFunctionLibrary::GetPathComponentRemainingLength(const UPathFollowingComponent* _pathFollowingComponent)
{
	JOY_EXITCONDITION_RET(!_pathFollowingComponent, 0.f, TEXT("Undefined PathFollowingComponent."));

	if (!_pathFollowingComponent->GetPath().IsValid() || _pathFollowingComponent->GetPath()->GetPathPoints().Num() == 0)
		return 0.f;

	AAIController* AIController = Cast<AAIController>(_pathFollowingComponent->GetOwner());
	return _pathFollowingComponent->GetPath()->GetLengthFromPosition(AIController->GetPawn()->GetActorLocation(), _pathFollowingComponent->GetNextPathIndex());
}

bool UJoyBlueprintFunctionLibrary::HasActorBegunPlay(const AActor* _actor)
{
	JOY_EXITCONDITION_RET(!_actor, false, TEXT("Unspecified Actor."));

	return _actor->HasActorBegunPlay();
}

FBox UJoyBlueprintFunctionLibrary::GetActorVisualBounds(const AActor* _actor)
{
	JOY_EXITCONDITION_RET(!_actor, FBox(), TEXT("Unspecified Actor."));

	TArray<UActorComponent*> staticMeshComponents = _actor->GetComponentsByClass(UStaticMeshComponent::StaticClass());
	if (staticMeshComponents.Num() == 0)
		return FBox(_actor->GetActorLocation(), _actor->GetActorLocation());
	
	FBox result = Cast<UStaticMeshComponent>(staticMeshComponents[0])->Bounds.GetBox();
	for (int32 i = 1; i < staticMeshComponents.Num(); ++i)
	{
		result = CombineBoxes(result, Cast<UStaticMeshComponent>(staticMeshComponents[i])->Bounds.GetBox());
	}
	return result;
}

FBox UJoyBlueprintFunctionLibrary::CombineBoxes(const FBox& _A, const FBox& _B)
{
	return FBox
	(
		FVector
		(
			FMath::Min(_A.Min.X, _B.Min.X),
			FMath::Min(_A.Min.Y, _B.Min.Y),
			FMath::Min(_A.Min.Z, _B.Min.Z)
		),
		FVector
		(
			FMath::Max(_A.Max.X, _B.Max.X),
			FMath::Max(_A.Max.Y, _B.Max.Y),
			FMath::Max(_A.Max.Z, _B.Max.Z)
		)
	);
}

FVector UJoyBlueprintFunctionLibrary::GetBoxCenter(const FBox& _box)
{
	return _box.GetCenter();
}

FVector UJoyBlueprintFunctionLibrary::GetBoxExtent(const FBox& _box)
{
	return _box.GetExtent();
}

float UJoyBlueprintFunctionLibrary::GetBoxRadius(const FBox& _box)
{
	return _box.GetExtent().Size();
}

FName UJoyBlueprintFunctionLibrary::CollisionProfileNameToName(const FCollisionProfileName& _collisionProfileName)
{
	return _collisionProfileName.Name;
}

FName UJoyBlueprintFunctionLibrary::GetLevelAssetName(const ULevel* _level)
{
	return _level->GetOuter()->GetFName();
}

FName UJoyBlueprintFunctionLibrary::GetLevelStreamingAssetName(const ULevelStreaming* _levelStreaming)
{
	FString baseFileName = FPaths::GetBaseFilename(_levelStreaming->GetWorldAssetPackageName());
	baseFileName.RemoveFromStart(_levelStreaming->GetWorld()->StreamingLevelsPrefix);
	return FName(*baseFileName);
}

bool UJoyBlueprintFunctionLibrary::IsWorldTearingDown(UObject* _worldContextObject)
{
	return _worldContextObject->GetWorld()->bIsTearingDown;
}

void UJoyBlueprintFunctionLibrary::DrawDebugCurve(UObject* _worldContextObject, UCurveFloat* _curve, ECoordinatesOrigin _coordinatesOrigin, FVector2D _position, FVector2D _size, float _curveXMin, float _curveXMax, float _curveValue, const FString& _curveName, FColor _curveColor , FColor _valueColor , int _samplesCount)
{
	if (!_curve)
		return;

	AActor* actor = Cast<AActor>(_worldContextObject);
	if (actor && !IsActorDebugTarget(actor))
		return;

	UJoyGameInstance* joyGameInstance = Cast<UJoyGameInstance>(UGameplayStatics::GetGameInstance(_worldContextObject));
	if (joyGameInstance)
	{
		joyGameInstance->DrawDebugCurve(_curve, _coordinatesOrigin, _position, _size, _curveXMin, _curveXMax, _curveValue, _curveName, _curveColor, _valueColor, _samplesCount);
	}
}

void UJoyBlueprintFunctionLibrary::LogError(UObject* WorldContextObject, const FString _errorMessage)
{
	LOG_ERROR(*FString(WorldContextObject->GetName() + " : ").Append(_errorMessage));
}

void UJoyBlueprintFunctionLibrary::LogWarning(UObject* WorldContextObject, const FString _warningMessage)
{
	LOG_WARNING(*FString(WorldContextObject->GetName() + " : ").Append(_warningMessage));
}

void UJoyBlueprintFunctionLibrary::DebugBreak()
{
	if (FGenericPlatformMisc::IsDebuggerPresent())
	{
		__debugbreak();
	}
}

AActor* UJoyBlueprintFunctionLibrary::BeginDeferredActorSpawnFromClass(const UObject* WorldContextObject, TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform, ESpawnActorCollisionHandlingMethod CollisionHandlingOverride /*= ESpawnActorCollisionHandlingMethod::Undefined*/, AActor* Owner /*= nullptr*/)
{
	return UGameplayStatics::BeginDeferredActorSpawnFromClass(WorldContextObject, ActorClass, SpawnTransform, CollisionHandlingOverride, Owner);
}

AActor* UJoyBlueprintFunctionLibrary::FinishSpawningActor(AActor* Actor, const FTransform& SpawnTransform)
{
	return UGameplayStatics::FinishSpawningActor(Actor, SpawnTransform);
}

