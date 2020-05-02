// Copyright 2019 Anatoli Kucharau. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputCoreTypes.h"
#include "TargetSelectionComponent.generated.h"

class USphereComponent;

/*A dispatcher called up when you enable or disable observation.
	@param bIsEnableTargetSelection On or off.
*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStateOfTargetSelection, bool, bIsEnableTargetSelection);

/*A dispatcher that is called up each time you switch to a new actor.
	@param ObservedActorNow The actor being watched now.
*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSwitchActorS, AActor*, ObservedActorNow);


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TARGETSELECTIONPLUGIN_API UTargetSelectionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UTargetSelectionComponent();

public:

	/*Do you want to sort the array of observed actors when begin watching?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TargetSelectionComponent")
		bool bIsSortArrayOfActors_WhenBegin;

	/*Do you want to sort the array of observed actors after switching between them?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TargetSelectionComponent")
		bool bIsSortArrayOfActors_WhenSwitch;

	/*Do you want to sort the array of observed actors when they remove?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TargetSelectionComponent")
		bool bIsSortArrayOfActors_WhenRemove;

	/*Do you want to sort the array of observed actors when they add?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TargetSelectionComponent")
		bool bIsSortArrayOfActors_WhenAddNew;

	/*Do you want to switch to the first actor in the array when the observed one remove? If false, switch to the next actor in the array.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TargetSelectionComponent")
		bool bIsSwitchToFirstActor_WhenRemoveObservedActor;

	/*
		Shall I activate the debug mode? (Print to message log.)
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TargetSelectionComponent")
		bool bIsDebugMode;

	/*
		Show collision (TargetSelectionCollision) in the game.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TargetSelectionComponent")
		bool bIsShowCollision;

	/*
	Do you check added actors for presence in the ObservedActorsArr array?
	It is recommended to enable it only if you add actors manually.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TargetSelectionComponent")
		bool bIsCheckAddingActorsForDuplicates;

	/*Declare the dispatcher to be called up when the observation is turned on or off.*/
	UPROPERTY(BlueprintAssignable, Category = "TargetSelectionComponent")
		FOnStateOfTargetSelection OnStateOfTargetSelection;

	/*Declare the dispatcher to be called when switching the observation to another actor.*/
	UPROPERTY(BlueprintAssignable, Category = "TargetSelectionComponent")
		FOnSwitchActorS OnSwitchActor;

private:
	/*The actor currently being observed.*/
	UPROPERTY(BlueprintGetter = GetObservedActor, Category = "TargetSelectionComponent")
		AActor* ObservedActor;

	/*Index of the currently observed ObservedActor actor in the ObservedActorsArr array.*/
	UPROPERTY(BlueprintGetter = GetIndexOfCurrentObservedActor, Category = "TargetSelectionComponent")
		int32 IndexOfCurrentObservedActor;

	/*An array of actors that can be observed.*/
	UPROPERTY(BlueprintGetter = GetObservedActorsArr, Category = "TargetSelectionComponent")
		TArray<AActor*> ObservedActorsArr;

	/*Collision for actor observation.*/
	UPROPERTY(EditAnywhere, BlueprintGetter = GetTargetSelectionCollision, Category = "TargetSelectionComponent")
		USphereComponent* TargetSelectionCollision;

	/*Is the observation in process now?*/
	UPROPERTY(BlueprintGetter = GetIsWatchingNow, Category = "TargetSelectionComponent")
		bool bIsWatchingNow;

	/*Uses an outside array?*/
	UPROPERTY(BlueprintGetter = GetIsCustomArray, Category = "TargetSelectionComponent")
		bool bIsCustomArray;

private:

	/*The current array of references to actor classes to be observed.*/
	TArray<TSubclassOf<AActor>> CurrentClassesFilter;

	/*The current array of references to classes to be ignored.*/
	TArray<TSubclassOf<AActor>> CurrentClassesFilterException;

	/*Current link to the interface class that inherits the actors to be observed.*/
	TSubclassOf<UInterface> CurrentInterfaceFilter;

	/*The current key that was pressed when the observation was activated.*/
	FKey CurrentInputKey;

	/*Is the filter valid by class?*/
	bool bIsValidClassesFilter;
	/*Is the filter valid by class for an exception?*/
	bool bIsValidClassesFilterException;
	/*Is the interface filter valid?*/
	bool bIsValidInterfaceFilter;


	/*Component owner.*/
	AActor* Owner;

	/*Copy of the outside array of actors.
	Used if	bIsCustomArray == true.
	Used in SortActorByFilters() when checking if an actor was in an input outside array.
	*/
	TArray<AActor*> CustomArrayDuplicate;

public:

	/*
		Watching the new actors. Must be triggered by pressing the observation key.
		@param ClassesFilter An array of references to the actor classes to be observed.
		@param ClassesFilterException An array of references to classes to be excluded from observation.
		@param InterfaceFilter Reference to the class of the interface that inherits the actors to be observed.
		@param IputKey Key pressed when observation is enabled. Allows you to set up observation of different actors by pressing different keys.
	*/
	UFUNCTION(BlueprintCallable, Category = "TargetSelectionComponent")
		void WatchActors(
			UPARAM(ref) TArray<TSubclassOf<AActor>>& ClassesFilter,
			UPARAM(ref) TArray<TSubclassOf<AActor>>& ClassesFilterException,
			TSubclassOf<UInterface> InterfaceFilter,
			FKey InputKey
		);

	/*
	Watching the new actors. Version with one filter by interface.
	Must be triggered by pressing the observation key.
	@param InterfaceFilter Reference to the class of the interface that inherits the actors to be observed.
	@param IputKey Key pressed when observation is enabled. Allows you to set up observation of different actors by pressing different keys.
	*/
	UFUNCTION(BlueprintCallable, Category = "TargetSelectionComponent")
		void WatchActors_OneFilter_Interface(
			TSubclassOf<UInterface> InterfaceFilter,
			FKey InputKey
		);

	/*Watching the new actors. Version with outside array. The component works with a copy of the input array.
		@param CustomArray is an outside array. The array is checked for fullness. Array actors are checked for validity.
		@param IputKey Key pressed when observation is enabled. Allows you to set up observation of different actors by pressing different keys.
	*/
	UFUNCTION(BlueprintCallable, Category = "TargetSelectionComponent")
		void WatchActors_CustomArray(
			UPARAM(ref) TArray<AActor*>& CustomArray,
			FKey InputKey
		);

	/*Turn off the observation. Must be triggered by pressing the disable observation key.*/
	UFUNCTION(BlueprintCallable, Category = "TargetSelectionComponent")
		void OffWatchingActors();

	/*
		Remove the actor from the array and switch to the next one. Must be triggered when the actor leaves the collision.
		@param LosedActor The actor what left the collision.
	*/
	UFUNCTION(BlueprintCallable, Category = "TargetSelectionComponent")
		void RemoveAndSwitchActors(AActor* RemovingActor);

	/*
		Add an actor to the array. Must be triggered when an actor enters a collision.
		@param FindedActor The actor that came into collision.
	*/
	UFUNCTION(BlueprintCallable, Category = "TargetSelectionComponent")
		void AddActor(AActor* NewActor);

	/*Get a pointer to the observed actor.*/
	UFUNCTION(BlueprintGetter, Category = "TargetSelectionComponent")
		AActor* GetObservedActor() const { return ObservedActor; };

	/*Get index of the currently observed ObservedActor actor in the ObservedActorsArr array.*/
	UFUNCTION(BlueprintGetter, Category = "TargetSelectionComponent")
		int32 GetIndexOfCurrentObservedActor() { return IndexOfCurrentObservedActor; };

	/*Get an array of actors that can be observed.*/
	UFUNCTION(BlueprintGetter, Category = "TargetSelectionComponent")
		TArray<AActor*> GetObservedActorsArr() { return ObservedActorsArr; };

	/*Get collision for actor observation.*/
	UFUNCTION(BlueprintGetter, Category = "TargetSelectionComponent")
		USphereComponent* GetTargetSelectionCollision() { return TargetSelectionCollision; };

	/*Get the current state - is it being observed now?*/
	UFUNCTION(BlueprintGetter, Category = "TargetSelectionComponent")
		bool GetIsWatchingNow() const { return bIsWatchingNow; };

	/*Get current state - uses an outside array?*/
	UFUNCTION(BlueprintGetter, Category = "TargetSelectionComponent")
		bool GetIsCustomArray() { return bIsCustomArray; };

	/*
	Assign the observed actor by the pointer. The actor must be in the ObservedActorsArr array.
	If it is not in the array, add it using the AddActor method first.
	NewObservedActor is checked for validity and presence in the array.
	*/
	UFUNCTION(BlueprintCallable, Category = "TargetSelectionComponent")
		void SetObservedActorByPointer(AActor* NewObservedActor);

	/*Assign the observed actor by index. The index is checked for validity.*/
	UFUNCTION(BlueprintCallable, Category = "TargetSelectionComponent")
		void SetObservedActorByIndex(int32 IndexOfNewObservedActor);



private:

	/*Check the key pressed.*/
	uint32 CheckInputData_InputKey(FKey InputKey);

	/*
	Check filter by class
		@param InClassesFilter An array of references to the actor classes to be observed.
		@param InClassesFilter An array of references to classes to be ignored from observation.
		@param LogValidClass Text for the log, if the class is not valid.
		@param LogEmptyArray Log Text if the class is empty.
		@param InbIsValidClassesFilter For validity.
	*/
	bool CheckInputData_Classes(
		TArray<TSubclassOf<AActor>>& InClassesFilter,
		TArray<TSubclassOf<AActor>>& InCurrentClassesFilter,
		FString LogNotValidClass,
		FString LogEmptyArray,
		bool& InbIsValidClassesFilter
	);

	/*Check the interface filter.*/
	bool CheckInputData_Interface(TSubclassOf<UInterface> InterfaceFilter);

	/*Switch between existing actors.*/
	bool SwitchCurrentActors();

	/*Switching to the first actor after switching on the observation mode.*/
	bool SwitchToNewActor();

	/*Call the IsObserved() method of the TargetSelectionInterface interface.*/
	void CallInterfaceIsObserved();

	/*Call the IsNotObserved() method of the TargetSelectionInterface interface.*/
	void CallInterfaceIsNotObserved();

	/*Take actors in the array ObservedActorsArr in the collision TargetSelectionCollision, including all filters.*/
	bool GetAvailableActors();

	/*Sort the actor by filters.*/
	bool SortActorByFilters(AActor* CurrentActor);

	/*Sorting the ObservedActorsArr array by the distance to the owner.*/
	void SortActorsByDistance();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;


};