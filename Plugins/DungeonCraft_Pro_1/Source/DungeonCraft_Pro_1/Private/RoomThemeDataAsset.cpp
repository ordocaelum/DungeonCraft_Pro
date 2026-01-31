// RoomThemeDataAsset.cpp
#include "RoomThemeDataAsset.h"

URoomThemeDataAsset::URoomThemeDataAsset()
{
    // Default constructor
    ThemeName = "Default Theme";
    ThemeDescription = FText::FromString("A standard room theme");
    SelectionWeight = 1.0f;

    // Default wall orientation
    bWallFacingX = true;

    // Default transform values
    FloorPivotOffset = FVector::ZeroVector;
    WallPivotOffset = FVector::ZeroVector;
    CeilingPivotOffset = FVector(0.0f, 0.0f, 400.0f);
}