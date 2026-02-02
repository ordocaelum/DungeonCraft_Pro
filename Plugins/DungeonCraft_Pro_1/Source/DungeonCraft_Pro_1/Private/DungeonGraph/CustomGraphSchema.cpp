#include "DungeonGraph/CustomGraphSchema.h"
#include "EdGraph/EdGraphPin.h"
#include "ToolMenus.h"
#include "GraphEditorActions.h"

const FPinConnectionResponse UCustomGraphSchema::CanCreateConnection(const UEdGraphPin* PinA, const UEdGraphPin* PinB) const
{
    if (PinA->PinType.PinCategory == PinB->PinType.PinCategory)
    {
        return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, TEXT("Connection allowed between matching pin types."));
    }

    return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Pin types must match."));
}

void UCustomGraphSchema::GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
    // Example action for creating a new node
    TSharedPtr<FEdGraphSchemaAction> NewNodeAction = MakeShared<FEdGraphSchemaAction_Dummy>(
        NSLOCTEXT("CustomGraph", "AddNode", "Add Node"),
        NSLOCTEXT("CustomGraph", "AddNodeTooltip", "Creates a new node in the graph."),
        NAME_None,
        0
    );
    ContextMenuBuilder.AddAction(NewNodeAction);
}

void UCustomGraphSchema::GetContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
    if (Context->Pin)
    {
        FToolMenuSection& Section = Menu->AddSection("CustomGraphSchemaPinActions", LOCTEXT("PinActionsMenuHeader", "Pin Actions"));
        if (Context->Pin->LinkedTo.Num() > 0)
        {
            Section.AddMenuEntry(FGraphEditorCommands::Get().BreakPinLinks);
        }
    }
    else if (Context->Node)
    {
        FToolMenuSection& Section = Menu->AddSection("CustomGraphSchemaNodeActions", LOCTEXT("NodeActionsMenuHeader", "Node Actions"));
        Section.AddMenuEntry(FGenericCommands::Get().Delete);
    }

    Super::GetContextMenuActions(Menu, Context);
}