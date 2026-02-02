#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphSchema.h"
#include "CustomGraphSchema.generated.h"

UCLASS()
class DUNGEONCRAFT_PRO_1_API UCustomGraphSchema : public UEdGraphSchema
{
    GENERATED_BODY()

public:
    /** Determines if a connection is allowed between two pins */
    virtual const FPinConnectionResponse CanCreateConnection(const UEdGraphPin* PinA, const UEdGraphPin* PinB) const override;

    /** Gets the graph context actions */
    virtual void GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const override;

    /** Provides context menu options */
    virtual void GetContextMenuActions(
        class UToolMenu* Menu,
        class UGraphNodeContextMenuContext* Context
    ) const override;
};