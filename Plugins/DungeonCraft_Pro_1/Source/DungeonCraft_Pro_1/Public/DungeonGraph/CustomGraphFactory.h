#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "CustomGraphFactory.generated.h"

UCLASS()
class DUNGEONCRAFT_PRO_1_API UCustomGraphFactory : public UFactory
{
    GENERATED_BODY()

public:
    // Constructor
    UCustomGraphFactory();

    // Creates a new instance of the UCustomGraph asset
    virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;

    // Makes this factory appear in the "Add New" menu in the Content Browser
    virtual bool ShouldShowInNewMenu() const override { return true; }
};