// Copyright 2019 Anatoli Kucharau. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TargetSelectionInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(Category = "TargetSelectionInterface", Blueprintable)
class UTargetSelectionInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class TARGETSELECTIONPLUGIN_API ITargetSelectionInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	/*Called from the actor when the actor begins to observe.*/
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "TargetSelectionInterface")
		void IsObserved();

	/*Called from the actor when the actor finishes observing.*/
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "TargetSelectionInterface")
		void IsNotObserved();

};
