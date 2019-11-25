// Fill out your copyright notice in the Description page of Project Settings.


#include "TargetSelectionComponent.h"
#include "TargetSelectionInterface.h"
#include "Containers/Array.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"


// Sets default values for this component's properties
UTargetSelectionComponent::UTargetSelectionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	/*Create and set up a collision*/
	TargetSelectionCollision = CreateDefaultSubobject<USphereComponent>(TEXT("TargetSelectionCollision"));
	TargetSelectionCollision->SetSphereRadius(1000);
	TargetSelectionCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TargetSelectionCollision->SetCollisionObjectType(ECC_Pawn);
	TargetSelectionCollision->SetCollisionResponseToAllChannels(ECR_Overlap);
	TargetSelectionCollision->SetCanEverAffectNavigation(false);
	TargetSelectionCollision->SetHiddenInGame(true);

	/*If the owner of the component is valid, then attach the collision to the owner.*/
	if (GetOwner() != nullptr)
	{
		Owner = GetOwner();
		TargetSelectionCollision->SetupAttachment(Owner->GetRootComponent());

	}

	bIsSortArrayOfActors_AtAll = true;
	bIsSortArrayOfActors_WhenManualSwitch = false;
	bIsSortArrayOfActors_WhenLoseCollision = false;
	bIsSortArrayOfActors_WhenFindCollision = false;
	bIsWatchingNow = false;
	bIsUseCollisionIfCustomArray = false;
	bIsDebugMode = false;
	bIsShowCollision = false;

	IndexOfCurrentObservedActor = 0;

	bIsValidClassesFilter = false;
	bIsValidClassesFilterException = false;
	bIsValidInterfaceFilter = false;

	bIsCustomArray = false;


	bIsSwitchToFirstActor_WhenLoseCollision = true;

}


// Called when the game starts
void UTargetSelectionComponent::BeginPlay()
{
	Super::BeginPlay();

	/*If the owner of the component is valid, then attach the collision to the owner. 
	It is done again in case the owner is not valid in the constructor.
	*/
	if (GetOwner() != nullptr)
	{
		Owner = GetOwner();
		if (!TargetSelectionCollision->IsAttachedTo(Owner->GetRootComponent()))
		{
			TargetSelectionCollision->SetupAttachment(Owner->GetRootComponent());
		}
	}

	if (bIsShowCollision)
	{
		/*Show collision in the game.*/
		TargetSelectionCollision->SetHiddenInGame(false);
	}

}

void UTargetSelectionComponent::WatchActors(
	UPARAM(ref) TArray<TSubclassOf<AActor>>& ClassesFilter,
	UPARAM(ref) TArray<TSubclassOf<AActor>>& ClassesFilterException,
	TSubclassOf<UInterface> InterfaceFilter,
	FKey InputKey
)
{
	/*Check the input key.*/
	uint32 StateOfCheckInputKey = CheckInputData_InputKey(InputKey);

	/*If the input isn't valid.*/
	if (StateOfCheckInputKey == 0)
	{
		return;
	}

	bIsCustomArray = false;

	/*If the input key is not equal to the temporary key, check the other input data.*/
	if (StateOfCheckInputKey == 2)
	{
		if (bIsDebugMode)
		{
			UE_LOG(LogTemp, Display, TEXT("TargetSelection: New key %s"), *InputKey.GetFName().ToString());
		}

		/*Disable observation.*/
		OffWatchingActors();

		/*Remember the key in the temporary variable.*/
		CurrentInputKey = InputKey;

		FString LogNotValidClass = "TargetSelection: Class in ClassesFilter is not valid, the Actor is used by default. (IsPendingKill or nullptr): ";
		FString LogEmptyArray = "TargetSelection: ClassesFilter is empty. The Actor is used by default.";
		CheckInputData_Classes(
			ClassesFilter,
			CurrentClassesFilter,
			LogNotValidClass,
			LogEmptyArray,
			bIsValidClassesFilter
		);

		LogNotValidClass = "TargetSelection: Class in ClassesFilterException is not valid, it won't be used. (IsPendingKill or nullptr): ";
		LogEmptyArray = "TargetSelection: ClassesFilterException is empty.";
		CheckInputData_Classes(
			ClassesFilterException,
			CurrentClassesFilterException,
			LogNotValidClass,
			LogEmptyArray,
			bIsValidClassesFilterException
		);

		CheckInputData_Interface(InterfaceFilter);

	}

	/*If the array of observed actors is not empty.*/
	if (ObservedActorsArr.Num() > 0)
	{
		SwitchCurrentActors();
	}
	/*If the array is empty.
	ObservedActorsArr.Num() == 0.*/
	else
	{
		/*Take actors to the array ObservedActorsArr.
		If the array is not empty, continue.*/
		if (GetAvailableActors())
		{
			/*Sort the array if allowed.*/
			if (bIsSortArrayOfActors_AtAll)
			{
				SortActorsByDistance();
			}
			SwitchToNewActor();
		}
	}
}

void UTargetSelectionComponent::WatchActors_OneFilter_Interface(TSubclassOf<UInterface> InterfaceFilter, FKey InputKey)
{
	/*Check the input key.*/
	uint32 StateOfCheckInputKey = CheckInputData_InputKey(InputKey);

	/*If the input isn't valid.*/
	if (StateOfCheckInputKey == 0)
	{
		return;
	}

	bIsCustomArray = false;

	/*If the input key is not equal to the time key, check the other input data.*/
	if (StateOfCheckInputKey == 2)
	{
		if (bIsDebugMode)
		{
			UE_LOG(LogTemp, Display, TEXT("TargetSelection: New key %s"), *InputKey.GetFName().ToString());
		}
		/*Disable observation.*/
		OffWatchingActors();

		/*Remember the key in the temporary variable.*/
		CurrentInputKey = InputKey;

		CheckInputData_Interface(InterfaceFilter);

	}

	/*If the array of observed actors is not empty.*/
	if (ObservedActorsArr.Num() > 0)
	{
		SwitchCurrentActors();
	}
	/*If the array is empty.
	ObservedActorsArr.Num() == 0.*/
	else
	{
		bIsValidClassesFilter = false;
		bIsValidClassesFilterException = false;
		/*Take actors to the array ObservedActorsArr.
		If the array is not empty, continue.*/
		if (GetAvailableActors())
		{
			/*Sort the array if allowed.*/
			if (bIsSortArrayOfActors_AtAll)
			{
				SortActorsByDistance();
			}
			SwitchToNewActor();
		}
	}
}

void UTargetSelectionComponent::WatchActors_CustomArray(
	UPARAM(ref) TArray<AActor*>& CustomArray,
	FKey InputKey
)
{

	if (CustomArray.Num() == 0)
	{
		if (bIsDebugMode)
		{
			UE_LOG(LogTemp, Warning, TEXT("TargetSelection: CustomArray is empty."))
		}

		return;
	}

	for (int32 Index = 0; Index != CustomArray.Num(); Index++)
	{
		if (CustomArray[Index] == nullptr)
		{
			if (bIsDebugMode)
			{
				UE_LOG(LogTemp, Warning, TEXT("TargetSelection: WatchActors_CustomArray(): Actor at Index %d in CustomArray is not valid."), Index);
			}
			return;
		}
	}

	/*Check the input key.*/
	uint32 StateOfCheckInputKey = CheckInputData_InputKey(InputKey);

	/*If the entrance isn't valid.*/
	if (StateOfCheckInputKey == 0)
	{
		return;
	}

	bIsCustomArray = true;

	/*If the input key is not equal to the time key.*/
	if (StateOfCheckInputKey == 2)
	{
		if (bIsDebugMode)
		{
			UE_LOG(LogTemp, Display, TEXT("TargetSelection: New key %s"), *InputKey.GetFName().ToString());
		}
		/*Disable observation.*/
		OffWatchingActors();

		/*Remember the key in the temporary variable.*/
		CurrentInputKey = InputKey;

	}

	/*If the array of observed actors is not empty.*/
	if (ObservedActorsArr.Num() > 0)
	{
		/*Switch the current actors.*/
		SwitchCurrentActors();
	}

	/*If the array is empty.
	ObservedActorsArr.Num() == 0.*/
	else
	{
		/*Copy the array if allowed. */
		if (bIsUseCollisionIfCustomArray)
		{
			CustomArrayDuplicate = CustomArray;
		}

		ObservedActorsArr = CustomArray;
		/*Sort the array if allowed.*/
		if (bIsSortArrayOfActors_AtAll)
		{
			SortActorsByDistance();
		}

		/*Turn on the observation, switch to the first actor.*/
		SwitchToNewActor();
	}
}

void UTargetSelectionComponent::OffWatchingActors()
{

	if (!bIsWatchingNow)
	{
		if (bIsDebugMode)
		{
			UE_LOG(LogTemp, Warning, TEXT("TargetSelection: OffWatchingActors(): TargetSelection is switched off previously."))
		}
		return;
	}

	if (ObservedActor == nullptr)
	{
		if (bIsDebugMode)
		{
			UE_LOG(LogTemp, Warning, TEXT("TargetSelection: OffWatchingActors(): ObservedActor is not valid."))
		}

		return;
	}

	CallInterfaceIsNotObserved();

	ObservedActor = nullptr;
	CurrentClassesFilter.Empty();
	CurrentClassesFilterException.Empty();
	CurrentInterfaceFilter = nullptr;
	FKey NullKey;
	CurrentInputKey = NullKey;
	ObservedActorsArr.Empty();

	bIsValidClassesFilter = false;
	bIsValidClassesFilterException = false;
	bIsValidInterfaceFilter = false;

	bIsWatchingNow = false;

	if (bIsCustomArray && bIsUseCollisionIfCustomArray)
	{
		CustomArrayDuplicate.Empty();
	}
	bIsCustomArray = false;


	OnStateOfTargetSelection.Broadcast(false);

	if (bIsDebugMode)
	{
		UE_LOG(LogTemp, Warning, TEXT("TargetSelection: OffWatchingActors(): TargetSelection is switched off."))
	}
}

void UTargetSelectionComponent::LoseAndSwitchActors(AActor* LosedActor)
{

	/*If the observation mode is disabled.*/
	if (!bIsWatchingNow)
	{
		if (bIsDebugMode)
		{
			UE_LOG(LogTemp, Warning, TEXT("TargetSelection: LoseAndSwitchActors(): TargetSelection not active."));
		}
		return;
	}

	if (LosedActor == nullptr)
	{
		if (bIsDebugMode)
		{
			UE_LOG(LogTemp, Warning, TEXT("TargetSelection: LoseAndSwitchActors(): Losed Actor is not valid."));
		}
		return;
	}

	if (bIsCustomArray && !bIsUseCollisionIfCustomArray)
	{
		if (bIsDebugMode)
		{
			UE_LOG(LogTemp, Warning, TEXT("TargetSelection: LoseAndSwitchActors(): Use custom array, auto switch don't work if bIsUseCollisionIfCustomArray == false."));
		}
		return;
	}

	/*If there is no actor in the array who came out of the collision.*/
	if (!ObservedActorsArr.Contains(LosedActor))
	{
		return;
	}

	/*If the observed actor is the same as the actor that came out.*/
	if (ObservedActor == LosedActor)
	{
		/*If there is one element in the array.*/
		if (ObservedActorsArr.Num() == 1)
		{
			if (bIsDebugMode)
			{
				UE_LOG(LogTemp, Warning, TEXT("TargetSelection: LoseAndSwitchActors(): Last Actor %s is living collision."), *LosedActor->GetName());
			}
			OffWatchingActors();
			return;
		}
		/*If there is more than 1 element in the array.*/
		else
		{
			/*Send a signal to the actor that he's not being observed.*/
			CallInterfaceIsNotObserved();
			if (bIsDebugMode)
			{
				UE_LOG(LogTemp, Warning, TEXT("TargetSelection: LoseAndSwitchActors(): %s is living collision."), *LosedActor->GetName());
			}

			/*Remove the observed actor from the array.*/
			ObservedActorsArr.RemoveAt(IndexOfCurrentObservedActor);

			/*If allowed, then sort it.*/
			if (bIsSortArrayOfActors_WhenLoseCollision)
			{
				SortActorsByDistance();
			}

			/*If allowed, then assign the index of the observed actor 0.*/
			if (bIsSwitchToFirstActor_WhenLoseCollision)
			{
				IndexOfCurrentObservedActor = 0;
			}
			else
			{

				--IndexOfCurrentObservedActor;

				/*If the current index is outside the array.*/
				if (IndexOfCurrentObservedActor > ObservedActorsArr.Num() - 1)
				{
					IndexOfCurrentObservedActor = 0;
				}
			}

			/*Determine the new observed actor by the new index.*/
			ObservedActor = ObservedActorsArr[IndexOfCurrentObservedActor];

			/*The actor is being observed.*/
			CallInterfaceIsObserved();

			OnSwitchActor.Broadcast(ObservedActor);

			if (bIsDebugMode)
			{
				if (ObservedActor != nullptr)
				{
					UE_LOG(LogTemp, Warning, TEXT("TargetSelection: LoseAndSwitchActors(): switch to %s."), *ObservedActor->GetName());
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("TargetSelection: LoseAndSwitchActors(): Observed Actor is not valid."));
				}
			}
		}
	}
	/*If the observed is not equal to the outgoing
	ObservedActor != LosedActor*/
	else
	{
		/*Remove it from the array.*/
		ObservedActorsArr.RemoveSingle(LosedActor);

		/*Find a new index for the actor being monitored.*/
		IndexOfCurrentObservedActor = ObservedActorsArr.Find(ObservedActor);

		if (bIsDebugMode)
		{
			UE_LOG(LogTemp, Warning, TEXT("TargetSelection: LoseAndSwitchActors(): %s is living collision."), *LosedActor->GetName());
		}
	}
}

void UTargetSelectionComponent::FindActors(AActor* FindedActor)
{
	/*If the observation mode is disabled.*/
	if (!bIsWatchingNow)
	{
		if (bIsDebugMode)
		{
			UE_LOG(LogTemp, Warning, TEXT("TargetSelection: FindActors(): TargetSelection not active."));
		}
		return;
	}

	/*If the actor found isn't valid.*/
	if (FindedActor == nullptr)
	{
		if (bIsDebugMode)
		{
			UE_LOG(LogTemp, Warning, TEXT("TargetSelection: FindActors(): Finded Actor is not valid."));
		}
		return;
	}

	/*If the outside array mode is enabled.*/
	if (bIsCustomArray && !bIsUseCollisionIfCustomArray)
	{
		if (bIsDebugMode)
		{
			UE_LOG(LogTemp, Warning, TEXT("TargetSelection: FindActors(): Use custom array, auto switch don't work if bIsUseCollisionIfCustomArray == false."));
		}
		return;
	}

	/*If the Actor didn't pass the filters.*/
	if (!SortActorByFilters(FindedActor))
	{
		return;
	}

	/*If the filter has passed, add the actor to the array.*/
	ObservedActorsArr.Add(FindedActor);

	if (bIsDebugMode)
	{
		UE_LOG(LogTemp, Warning, TEXT("TargetSelection: FindActors(): %s is find collision."), *FindedActor->GetName());
	}

	/*If sorting is allowed.*/
	if (bIsSortArrayOfActors_WhenFindCollision)
	{
		SortActorsByDistance();
	}
}

uint32 UTargetSelectionComponent::CheckInputData_InputKey(FKey InputKey)
{
	/*If the input key is not valid.*/
	if (!InputKey.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("TargetSelection: Input key is not valid."));
		return 0;
	}

	/*If the temporary key is valid.*/
	if (CurrentInputKey.IsValid())
	{
		/*If the temporary and input keys are equal, the input data have already been checked.*/
		if (CurrentInputKey == InputKey)
		{
			/*Return the validity of the data.*/
			return 1;
		}
		/*If the temporary and the input aren't equal.
		CurrentInputKey != InputKey*/
		else
		{
			return 2;
		}
	}

	return 2;
}

bool UTargetSelectionComponent::CheckInputData_Classes(
	TArray<TSubclassOf<AActor>>& InClassesFilter,
	TArray<TSubclassOf<AActor>>& InCurrentClassesFilter,
	FString LogNotValidClass,
	FString LogEmptyArray,
	bool& InbIsValidClassesFilter
)
{
	/*If the filter array by class is not empty.*/
	if (InClassesFilter.Num() > 0)
	{
		if (bIsDebugMode)
		{
			UE_LOG(LogTemp, Warning, TEXT("TargetSelection: Size of array of ClassesFilter: %d"), InClassesFilter.Num());
		}

		/*Let's say the filter is valid.*/
		InbIsValidClassesFilter = true;
		/*Filter array scan.*/
		for (auto& CurrentClass : InClassesFilter)
		{
			/*If the class is not valid or is ready to be removed.*/
			if (CurrentClass != nullptr)
			{
				if (CurrentClass->IsPendingKill())
				{
					InbIsValidClassesFilter = false;
					if (bIsDebugMode)
					{
						UE_LOG(LogTemp, Warning, TEXT("%s%s"), *LogNotValidClass, *CurrentClass->GetName());
					}
				}
			}
			else
			{
				InbIsValidClassesFilter = false;
				if (bIsDebugMode)
				{
					UE_LOG(LogTemp, Warning, TEXT("%s nullptr"), *LogNotValidClass);
				}
			}
		}
		/*If the filter is valid.*/
		if (InbIsValidClassesFilter)
		{
			/*Remember the array into a temporary variable.*/
			InCurrentClassesFilter = InClassesFilter;
		}
		/*If the filter is not valid.
		!InbIsValidClassesFilter*/
		else
		{
			/*Clear the temporary array.*/
			InCurrentClassesFilter.Empty();
		}
	}
	/*If the array of filters by class is empty.
	InClassesFilter.Num() == 0 */
	else
	{
		InbIsValidClassesFilter = false;
		/*Clear the temporary array by class.*/
		InCurrentClassesFilter.Empty();
		if (bIsDebugMode)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s"), *LogEmptyArray);
		}
	}

	return true;

}

bool UTargetSelectionComponent::CheckInputData_Interface(TSubclassOf<UInterface> InterfaceFilter)
{
	/*If the interface filter is valid.*/
	if (InterfaceFilter != nullptr)
	{
		bIsValidInterfaceFilter = true;

		/*Remember in the time variable.*/
		CurrentInterfaceFilter = InterfaceFilter;

		return true;
	}
	/*If the interface filter is not valid.
	InterfaceFilter == nullptr*/
	else
	{
		bIsValidInterfaceFilter = false;
		if (bIsDebugMode)
		{
			UE_LOG(LogTemp, Warning, TEXT("TargetSelection: InterfaceFilter is not valid."));
		}

		return false;
	}

	return false;
}

bool UTargetSelectionComponent::SwitchCurrentActors()
{
	/*If there is only 1 element in the array.*/
	if (ObservedActorsArr.Num() == 1)
	{
		if (bIsDebugMode)
		{
			UE_LOG(LogTemp, Display, TEXT("TargetSelection: No switch, stay on %s"), *ObservedActor->GetName());
		}
		/*Don't do anything and get out.*/
		return true;
	}
	/*If there is more than 1 element in the array.
	ObservedActorsArr.Num() > 1.*/
	else
	{
		/*Call the IsNotObserved() interface method.*/
		CallInterfaceIsNotObserved();

		if (bIsDebugMode)
		{
			UE_LOG(LogTemp, Display, TEXT("TargetSelection: Switch from %s"), *ObservedActor->GetName());
		}

		/*If the current index is less than the maximum index.*/
		if (IndexOfCurrentObservedActor < ObservedActorsArr.Num() - 1)
		{
			/*Increase the index by 1.*/
			++IndexOfCurrentObservedActor;
		}
		/*If the current index is equal to or greater than the maximum index.*/
		else
		{
			/*Zero it.*/
			IndexOfCurrentObservedActor = 0;
		}
		/*Change the actor to be observed.*/
		ObservedActor = ObservedActorsArr[IndexOfCurrentObservedActor];

		/*Call the IsObserved() interface method.*/
		CallInterfaceIsObserved();

		/*Call up the switching dispatcher.*/
		OnSwitchActor.Broadcast(ObservedActor);

		/*Sort the array if allowed.*/
		if (bIsSortArrayOfActors_AtAll && bIsSortArrayOfActors_WhenManualSwitch)
		{
			SortActorsByDistance();
		}

		if (bIsDebugMode)
		{
			if (ObservedActor != nullptr)
			{
				UE_LOG(LogTemp, Display, TEXT("TargetSelection: to %s"), *ObservedActor->GetName());
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("TargetSelection: Observed Actor is not valid."));
			}

		}
	}

	return true;
}

bool UTargetSelectionComponent::SwitchToNewActor()
{

	/*Identify the actor to be observed.*/
	ObservedActor = ObservedActorsArr[0];

	/*Determine the index of the observed actor in the array.*/
	IndexOfCurrentObservedActor = 0;

	/*Call the IsObserved() interface method.*/
	CallInterfaceIsObserved();

	/*Call up the switching dispatcher.*/
	OnSwitchActor.Broadcast(ObservedActor);

	/*Indicate the state of observation.*/
	bIsWatchingNow = true;

	/*Call the dispatcher for observation.*/
	OnStateOfTargetSelection.Broadcast(bIsWatchingNow);

	if (bIsDebugMode)
	{
		UE_LOG(LogTemp, Display, TEXT("TargetSelection: First switch to %s"), *ObservedActor->GetName());
	}

	return true;
}

void UTargetSelectionComponent::CallInterfaceIsObserved()
{
	/*If the actor is valid.*/
	if (ObservedActor != nullptr)
	{
		/*If the interface is valid.*/
		if (ObservedActor->GetClass()->ImplementsInterface(UTargetSelectionInterface::StaticClass()))
		{
			/*Call the signal that the actor is being observed.*/
			ITargetSelectionInterface::Execute_IsObserved(ObservedActor);
		}
		else
		{
			if (bIsDebugMode)
			{
				UE_LOG(LogTemp, Error, TEXT("TargetSelection: CallInterfaceIsObserved(): ObservedActor does not implements TargetSelectionInterface."));
			}
		}

	}
	else
	{
		if (bIsDebugMode)
		{
			UE_LOG(LogTemp, Error, TEXT("TargetSelection: CallInterfaceIsObserved(): ObservedActor is not valid."));
		}
	}
}

void UTargetSelectionComponent::CallInterfaceIsNotObserved()
{
	/*If the actor is valid.*/
	if (ObservedActor != nullptr)
	{
		/*If the interface is valid.*/
		if (ObservedActor->GetClass()->ImplementsInterface(UTargetSelectionInterface::StaticClass()))
		{
			/*Call the signal that the actor is not being observed.*/
			ITargetSelectionInterface::Execute_IsNotObserved(ObservedActor);
		}
		else
		{
			if (bIsDebugMode)
			{
				UE_LOG(LogTemp, Error, TEXT("TargetSelection: CallInterfaceIsNotObserved(): ObservedActor does not implements TargetSelectionInterface."));
			}
		}

	}
	else
	{
		if (bIsDebugMode)
		{
			UE_LOG(LogTemp, Error, TEXT("TargetSelection: CallInterfaceIsNotObserved(): ObservedActor is not valid."));
		}
	}
}

bool UTargetSelectionComponent::GetAvailableActors()
{
	/*Temporary array.*/
	TArray<AActor*> TempArrayOfActors;

	/*Take the actors to the temporary array.*/
	TargetSelectionCollision->GetOverlappingActors(TempArrayOfActors);

	/*Scans an array of actors.*/
	for (auto& CurrentActor : TempArrayOfActors)
	{
		if (SortActorByFilters(CurrentActor))
		{
			/*If the filter has passed, add the actor to the array.*/
			ObservedActorsArr.Add(CurrentActor);
		}

	}

	if (ObservedActorsArr.Num() == 0)
	{
		if (bIsDebugMode)
		{
			UE_LOG(LogTemp, Warning, TEXT("TargetSelection: GetAvailableActors(): No Actors in ObservedActorsArr."));
		}
		return false;
	}

	return true;
}

bool UTargetSelectionComponent::SortActorByFilters(AActor* CurrentActor)
{
	/*If allowed, go through the copy of the outside array and find matches with CurrentActor.*/
	if (bIsCustomArray && bIsUseCollisionIfCustomArray)
	{
		bool bIsValidCustomArrayFilter = false;
		for (auto& CurrentActorDuplicate : CustomArrayDuplicate)
		{
			if (CurrentActor == CurrentActorDuplicate)
			{
				bIsValidCustomArrayFilter = true;
				break;
			}
		}

		return bIsValidCustomArrayFilter;
	}

	if (CurrentActor == nullptr)
	{
		if (bIsDebugMode)
		{
			UE_LOG(LogTemp, Warning, TEXT("TargetSelection: SortActorByFilters(): CurrentActor is not valid."));
		}
		return false;
	}
	/*If the filter is valid.*/
	if (bIsValidClassesFilter)
	{
		/*Suppose that the filter array does not contain the CurrentActor actor class.*/
		bool bIsClassesFilterWorkOut = false;
		/*Scan an array of filters.*/
		for (auto& CurrenClass : CurrentClassesFilter)
		{
			/*If the class matches or is a child filter, remember the result and exit the loop.*/
			if (UKismetMathLibrary::ClassIsChildOf(CurrentActor->GetClass(), CurrenClass))
			{
				bIsClassesFilterWorkOut = true;
				break;
			}
		}
		/*If the filter's triggered, get out.*/
		if (!bIsClassesFilterWorkOut)
		{
			return false;
		}
	}

	/*If the filter is valid.*/
	if (bIsValidClassesFilterException)
	{
		/*Suppose that the filter array does not contain the CurrentActor actor class.*/
		bool bIsClassesFilterExceptionWorkOut = false;
		/*Scan an array of filters.*/
		for (auto& CurrenClassException : CurrentClassesFilterException)
		{
			/*If the class matches or is a child filter, remember the result and exit the loop.*/
			if (UKismetMathLibrary::ClassIsChildOf(CurrentActor->GetClass(), CurrenClassException))
			{
				bIsClassesFilterExceptionWorkOut = true;
				break;
			}
		}
		/*If the filter's triggered, get out.*/
		if (bIsClassesFilterExceptionWorkOut)
		{
			return false;
		}
	}

	/*If the filter is valid.*/
	if (bIsValidInterfaceFilter)
	{
		if (!UKismetSystemLibrary::DoesImplementInterface(CurrentActor, CurrentInterfaceFilter))
		{
			return false;
		}
	}

	return true;

}

void UTargetSelectionComponent::SortActorsByDistance()
{
	/*The lambda requires a local variable in this method.*/
	AActor* MyOwner = Owner;
	/*If the ravener is valid and in the array is more than 1 element.*/
	if (MyOwner != nullptr && ObservedActorsArr.Num() > 1)
	{
		ObservedActorsArr.Sort([MyOwner](const AActor& A, const AActor& B)
		{
			return A.GetDistanceTo(MyOwner) < B.GetDistanceTo(MyOwner);
		});
	}
	else
	{
		if (MyOwner == nullptr)
		{
			if (bIsDebugMode)
			{
				UE_LOG(LogTemp, Error, TEXT("TargetSelection: GetOwner() == nullptr"));
			}
		}
	}
}



