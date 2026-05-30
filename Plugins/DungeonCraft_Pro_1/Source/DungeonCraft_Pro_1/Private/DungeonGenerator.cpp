// DungeonGenerator.cpp
// BR_BETRAYAL_2025 - Enhanced dungeon generator with replication support

#include "DungeonGenerator.h"
#include "DungeonConfigDataAsset.h"
#include "RoomThemeDataAsset.h" // Include the new room theme system
#include "DrawDebugHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"
#include "Components/DecalComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY(DungeonGenerator);

const FName ADungeonGenerator::DUNGEON_MESH_TAG = FName("BR_BETRAYAL_DungeonTile");
const FName ADungeonGenerator::CEILING_MESH_TAG = FName("BR_BETRAYAL_CeilingTile");
const FName ADungeonGenerator::PILLAR_MESH_TAG = FName("BR_BETRAYAL_Pillar"); //kept only for backward compatability
const FName ADungeonGenerator::PROP_MESH_TAG = FName("BR_BETRAYAL_Prop");
const FName ADungeonGenerator::DUNGEON_BP_TAG = FName("DungeonBP");

// Implementation of the ClearDungeon function
void ADungeonGenerator::ClearDungeon()
{
    UE_LOG(LogTemp, Warning, TEXT("Clear Dungeon button pressed - performing thorough cleanup"));

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get World reference in ClearDungeon"));
        return;
    }

    // ADD THIS DEBUGGING SECTION:
    UE_LOG(LogTemp, Warning, TEXT("ClearDungeon: Pre-cleanup check for actors with DUNGEON_BP_TAG (%s)"), *DUNGEON_BP_TAG.ToString());
    TArray<AActor*> AllActorsInWorld;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActorsInWorld); // Consider a more specific base class if your BPs derive from one
    int32 FoundWithTagPreCheck = 0;
    for (AActor* ActorToCheck : AllActorsInWorld)
    {
        if (ActorToCheck) // Basic null check
        {
            // Check if it's potentially one of your BPs - you might need a more specific check here
            // For now, let's just check tags on all actors that might be relevant
            if (ActorToCheck->Tags.Contains(DUNGEON_BP_TAG))
            {
                UE_LOG(LogTemp, Warning, TEXT("ClearDungeon (Pre-Check): Actor %s HAS the tag %s. Tags: [%s]"),
                    *ActorToCheck->GetName(),
                    *DUNGEON_BP_TAG.ToString(),
                    *FString::JoinBy(ActorToCheck->Tags, TEXT(", "), [](const FName& Tag) { return Tag.ToString(); }));
                FoundWithTagPreCheck++;
            }
            // Optional: If you know part of the name your BPs usually have, you can log those that don't have the tag
            // else if (ActorToCheck->GetName().Contains(TEXT("ExpectedBPNamePart"))) 
            // {
            //     UE_LOG(LogTemp, Warning, TEXT("ClearDungeon (Pre-Check): Actor %s (suspected BP) does NOT have DUNGEON_BP_TAG. Current tags: [%s]"), 
            //            *ActorToCheck->GetName(), 
            //            *FString::JoinBy(ActorToCheck->Tags, TEXT(", "), [](const FName& Tag){ return Tag.ToString(); }));
            // }
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("ClearDungeon (Pre-Check): Found %d actors with the tag %s via manual iteration."), FoundWithTagPreCheck, *DUNGEON_BP_TAG.ToString());
    // END OF ADDED DEBUGGING SECTION

    // Arrays to store actors we need to destroy
    TArray<AActor*> DungeonMeshes;
    TArray<AActor*> CeilingMeshes;
    TArray<AActor*> PillarMeshes;
    TArray<AActor*> PropMeshes;
    TArray<AActor*> BlueprintActors; // This is the one we're interested in

    // Find all actors with our tags
    UGameplayStatics::GetAllActorsWithTag(World, DUNGEON_MESH_TAG, DungeonMeshes);
    UGameplayStatics::GetAllActorsWithTag(World, CEILING_MESH_TAG, CeilingMeshes);
    UGameplayStatics::GetAllActorsWithTag(World, PILLAR_MESH_TAG, PillarMeshes);
    UGameplayStatics::GetAllActorsWithTag(World, PROP_MESH_TAG, PropMeshes);
    UGameplayStatics::GetAllActorsWithTag(World, DUNGEON_BP_TAG, BlueprintActors); // This is the call that's currently finding 0

    // Log statistics (this is your existing log)
    UE_LOG(LogTemp, Warning, TEXT("Found: %d mesh actors, %d ceiling meshes, %d pillars, %d props, %d blueprint actors (via GetAllActorsWithTag)"),
        DungeonMeshes.Num(), CeilingMeshes.Num(), PillarMeshes.Num(), PropMeshes.Num(), BlueprintActors.Num());

    // Additional blueprint actor discovery if none found by tag
    if (BlueprintActors.Num() == 0)
    {
        // Get all actors in the world
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);

        // Correct way to check for "DungeonBP" in name
        for (AActor* Actor : AllActors)
        {
            if (Actor && Actor->GetName().Contains(TEXT("DungeonBP")))
            {
                BlueprintActors.AddUnique(Actor);
            }
        }

        UE_LOG(LogTemp, Warning, TEXT("Found %d additional blueprint actors by name search"),
            BlueprintActors.Num());
    }

    // Destroy all actors
    int32 TotalDestroyed = 0;

    for (AActor* Actor : DungeonMeshes)
    {
        if (Actor)
        {
            Actor->Destroy();
            TotalDestroyed++;
        }
    }

    for (AActor* Actor : CeilingMeshes)
    {
        if (Actor)
        {
            Actor->Destroy();
            TotalDestroyed++;
        }
    }

    for (AActor* Actor : PillarMeshes)
    {
        if (Actor)
        {
            Actor->Destroy();
            TotalDestroyed++;
        }
    }

    for (AActor* Actor : PropMeshes)
    {
        if (Actor)
        {
            Actor->Destroy();
            TotalDestroyed++;
        }
    }

    for (AActor* Actor : BlueprintActors)
    {
        if (Actor)
        {
            UE_LOG(LogTemp, Warning, TEXT("Destroying blueprint actor: %s"), *Actor->GetName());
            Actor->Destroy();
            TotalDestroyed++;
        }
    }

    // Reset data structures
    GeneratedRooms.Empty();
    CorridorFloorLocations.Empty();
    CorridorWalls.Empty();
    CorridorCorners.Empty();

    UE_LOG(LogTemp, Warning, TEXT("Dungeon cleanup complete: Destroyed %d total actors"), TotalDestroyed);

    // Force immediate garbage collection for clean slate
    if (GEngine)
    {
        GEngine->ForceGarbageCollection(true);
    }
}

// Sets default values
ADungeonGenerator::ADungeonGenerator()
{
    // Structure needs to be replicated for multiplayer
    bReplicates = true;

    // Set this actor to call Tick() every frame (can be disabled for performance)
    PrimaryActorTick.bCanEverTick = false;

    // Create a random seed by default
    DungeonSeed = FMath::Rand();

    // Set default values for generation parameters
    MinRoomSize = 3;
    MaxRoomSize = 6;
    RoomsToGenerate = 5;
    MaxRandomAttemptsPerRoom = 20;
    TileMapRows = 30;
    TileMapColumns = 30;
    FloorTileSize = 400.0f;
    WallWidth = 20.0f;

    // Default colors for debug visualization
    DefaultFloorSpawnLocationColor = FLinearColor(0.0f, 1.0f, 0.0f, 0.5f); // Green
    OffsetedFloorSpawnLocationColor = FLinearColor(0.0f, 0.5f, 0.0f, 0.5f); // Dark Green
    DefaultWallSpawnLocationColor = FLinearColor(1.0f, 0.0f, 0.0f, 0.5f); // Red
    OffsetedWallSpawnLocationColor = FLinearColor(0.5f, 0.0f, 0.0f, 0.5f); // Dark Red
}

void ADungeonGenerator::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // Replicate the dungeon seed to ensure clients generate the same dungeon
    DOREPLIFETIME(ADungeonGenerator, DungeonSeed);
}

// Called when the game starts or when spawned
void ADungeonGenerator::BeginPlay()
{
    Super::BeginPlay();

    // Only server should initiate generation if we're in a multiplayer game
    if (HasAuthority() && bGenerateOnBeginPlay)
    {
        GenerateDungeon();
    }
}

void ADungeonGenerator::OnRep_DungeonSeed()
{
    // When seed is received by clients, they can generate the dungeon
    if (!HasAuthority() && bGenerateOnBeginPlay)
    {
        GenerateDungeon();
    }
}

float ADungeonGenerator::CalculateFloorTileSize(const UStaticMesh& Mesh) const
{
    // Calculate tile size based on mesh bounds
    // For a 400x400 mesh, this should return 400
    return FMath::Abs(Mesh.GetBoundingBox().Min.Y) + FMath::Abs(Mesh.GetBoundingBox().Max.Y);
}

FRotator ADungeonGenerator::CalculateWallRotation(bool bWallFacingXProperty, const FWallSpawnPoint& WallSpawnPoint, const FVector& WallPivotOffsetOverride, FVector& LocationOffset) const
{
    FRotator WallRotation = FRotator::ZeroRotator;
    LocationOffset = FVector::ZeroVector;

    //If the point is generated in a way that is looking at the X axis and the wall is rotated to look at Y make sure to 
    //rotate the wall and apply an offset
    //Note: Points looking at X axis are spread along Y
    //WallSpawnPoint.bFacingX = true when the wall is located in an "up/down" tile
    if (!bWallFacingXProperty && WallSpawnPoint.bFacingX)
    {
        WallRotation = FRotator(0.f, -90.f, 0.f);
        LocationOffset.Y += FMath::Abs(WallPivotOffsetOverride.X);
    }
    else if (!WallSpawnPoint.bFacingX && bWallFacingXProperty)
    {
        WallRotation = FRotator(0.f, -90.f, 0.f);
    }
    else //No rotation adjustments needed; just apply the original offset
    {
        LocationOffset += WallPivotOffsetOverride;
    }

    return WallRotation;
}

void ADungeonGenerator::InitTileMatrix()
{
    // Reset the tile matrix to empty state
    TileMatrix.Empty(TileMapRows);

    // Initialize with all tiles unoccupied
    for (int32 i = 0; i < TileMapRows; i++)
    {
        TArray<FTile> Row;
        Row.SetNum(TileMapColumns);

        for (int32 j = 0; j < TileMapColumns; j++)
        {
            Row[j] = FTile(i, j);
        }

        TileMatrix.Add(Row);
    }

    UE_LOG(DungeonGenerator, Log, TEXT("Initialized tile matrix with %d rows and %d columns"), TileMapRows, TileMapColumns);
}

bool ADungeonGenerator::IsTileOccupied(int32 Row, int32 Column) const
{
    // Check if coordinates are valid
    if (Row >= 0 && Row < TileMatrix.Num() && Column >= 0 && Column < TileMatrix[Row].Num())
    {
        return TileMatrix[Row][Column].bIsRoom || TileMatrix[Row][Column].RoomID >= 0;
    }

    // Out of bounds counts as occupied
    return true;
}

bool ADungeonGenerator::CanPlaceRoom(int32 StartRow, int32 StartCol, int32 Width, int32 Height) const
{
    // Check if all tiles in the room are available
    for (int32 i = StartRow; i < StartRow + Height; i++)
    {
        for (int32 j = StartCol; j < StartCol + Width; j++)
        {
            if (i < 0 || i >= TileMapRows || j < 0 || j >= TileMapColumns || IsTileOccupied(i, j))
            {
                return false;
            }
        }
    }

    return true;
}

void ADungeonGenerator::OccupyTiles(int32 StartRow, int32 StartCol, int32 Width, int32 Height, int32 RoomID)
{
    for (int32 i = StartRow; i < StartRow + Height; i++)
    {
        for (int32 j = StartCol; j < StartCol + Width; j++)
        {
            if (i >= 0 && i < TileMapRows && j >= 0 && j < TileMapColumns)
            {
                TileMatrix[i][j].bIsRoom = true;
                TileMatrix[i][j].RoomID = RoomID;
            }
        }
    }
}

void ADungeonGenerator::GenerateRooms()
{
    // Set random seed for deterministic generation
    FMath::RandInit(DungeonSeed);

    // Clear any existing rooms
    GeneratedRooms.Empty();

    for (int32 RoomIndex = 0; RoomIndex < RoomsToGenerate; RoomIndex++)
    {
        bool bRoomPlaced = false;

        for (int32 Attempt = 0; Attempt < MaxRandomAttemptsPerRoom && !bRoomPlaced; Attempt++)
        {
            // Generate random room size within constraints
            int32 RoomWidth = FMath::RandRange(MinRoomSize, MaxRoomSize);
            int32 RoomHeight = FMath::RandRange(MinRoomSize, MaxRoomSize);

            // Generate random starting position
            int32 StartRow = FMath::RandRange(0, TileMapRows - RoomWidth - 1);
            int32 StartCol = FMath::RandRange(0, TileMapColumns - RoomHeight - 1);

            // Check if room can be placed
            if (CanPlaceRoom(StartRow, StartCol, RoomWidth, RoomHeight))
            {
                // Occupy tiles for this room
                OccupyTiles(StartRow, StartCol, RoomWidth, RoomHeight, RoomIndex);

                // Create a new room
                FRoom NewRoom;

                // Calculate floor tile positions
                for (int32 i = StartRow; i < StartRow + RoomHeight; i++)
                {
                    for (int32 j = StartCol; j < StartCol + RoomWidth; j++)
                    {
                        FVector FloorPos = FVector(i * FloorTileSize, j * FloorTileSize, 0.0f);
                        NewRoom.FloorTileWorldLocations.Add(FloorPos);
                    }
                }

                GeneratedRooms.Add(NewRoom);
                bRoomPlaced = true;

                UE_LOG(DungeonGenerator, Log, TEXT("Placed room %d (Size: %dx%d) at [%d,%d] after %d attempts"),
                    RoomIndex, RoomWidth, RoomHeight, StartRow, StartCol, Attempt + 1);
            }
        }

        if (!bRoomPlaced)
        {
            UE_LOG(DungeonGenerator, Warning, TEXT("Failed to place room %d after %d attempts"),
                RoomIndex, MaxRandomAttemptsPerRoom);
        }
    }

    UE_LOG(DungeonGenerator, Log, TEXT("Generated %d rooms"), GeneratedRooms.Num());

    // After rooms are generated, assign themes

    // First, calculate total weight of all themes
    float TotalWeight = DefaultThemeWeight; // Weight for default theme
    for (URoomThemeDataAsset* Theme : RoomThemes)
    {
        if (Theme)
        {
            TotalWeight += Theme->SelectionWeight;
        }
    }

    // Create array of room indices to randomize special room selection
    TArray<int32> RoomIndices;
    for (int32 i = 0; i < GeneratedRooms.Num(); i++)
    {
        RoomIndices.Add(i);
    }

    // Shuffle indices
    for (int32 i = 0; i < RoomIndices.Num(); i++)
    {
        int32 SwapIndex = FMath::RandRange(i, RoomIndices.Num() - 1);
        RoomIndices.Swap(i, SwapIndex);
    }

    // Calculate how many special rooms to create (limited by MaxSpecialRooms)
    int32 ValidThemes = 0;
    for (URoomThemeDataAsset* Theme : RoomThemes)
    {
        if (Theme) ValidThemes++;
    }
    int32 SpecialRoomsToCreate = FMath::Min(MaxSpecialRooms, FMath::Min(ValidThemes, GeneratedRooms.Num()));

    // Assign themes to first N rooms in our shuffled list
    int32 ValidThemeIndex = 0;
    for (int32 i = 0; i < SpecialRoomsToCreate; i++)
    {
        int32 RoomIdx = RoomIndices[i];

        // Find next valid theme
        while (ValidThemeIndex < RoomThemes.Num() && !RoomThemes[ValidThemeIndex])
        {
            ValidThemeIndex++;
        }

        // Assign theme if valid
        if (ValidThemeIndex < RoomThemes.Num())
        {
            GeneratedRooms[RoomIdx].RoomTheme = RoomThemes[ValidThemeIndex];
            ValidThemeIndex++;
        }

        // Calculate room grid dimensions for organized prop placement
        CalculateRoomGridDimensions(GeneratedRooms[RoomIdx]);
    }

    // Calculate grid dimensions for remaining rooms too
    for (int32 i = SpecialRoomsToCreate; i < GeneratedRooms.Num(); i++)
    {
        if (RoomIndices.IsValidIndex(i))
        {
            int32 RoomIdx = RoomIndices[i];
            CalculateRoomGridDimensions(GeneratedRooms[RoomIdx]);
        }
    }
}

// Helper function to calculate room dimensions
void ADungeonGenerator::CalculateRoomGridDimensions(FRoom& Room)
{
    // Find min/max grid coordinates
    Room.MinGridX = MAX_int32;
    Room.MinGridY = MAX_int32;
    int32 MaxGridX = MIN_int32;
    int32 MaxGridY = MIN_int32;

    for (const FVector& Pos : Room.FloorTileWorldLocations)
    {
        int32 GridX = FMath::FloorToInt(Pos.X / FloorTileSize);
        int32 GridY = FMath::FloorToInt(Pos.Y / FloorTileSize);

        Room.MinGridX = FMath::Min(Room.MinGridX, GridX);
        Room.MinGridY = FMath::Min(Room.MinGridY, GridY);
        MaxGridX = FMath::Max(MaxGridX, GridX);
        MaxGridY = FMath::Max(MaxGridY, GridY);
    }

    Room.RoomWidth = MaxGridX - Room.MinGridX + 1;
    Room.RoomHeight = MaxGridY - Room.MinGridY + 1;
}

void ADungeonGenerator::EnsureWallSpawnPoints()
{
    // Clear any existing wall spawn points first
    for (int32 RoomIdx = 0; RoomIdx < GeneratedRooms.Num(); RoomIdx++)
    {
        GeneratedRooms[RoomIdx].WallSpawnPoints.Empty();
    }
    CorridorWalls.Empty();

    // Process rooms to generate walls
    for (int32 RoomIdx = 0; RoomIdx < GeneratedRooms.Num(); RoomIdx++)
    {
        TArray<FWallSpawnPoint> RoomWalls;
        const FRoom& Room = GeneratedRooms[RoomIdx];

        for (const FVector& FloorPos : Room.FloorTileWorldLocations)
        {
            AddWallsAroundTile(FloorPos, RoomWalls);
        }

        // Assign the generated walls to the room
        GeneratedRooms[RoomIdx].WallSpawnPoints = RoomWalls;

        UE_LOG(DungeonGenerator, Log, TEXT("Room %d has %d wall spawn points"),
            RoomIdx, GeneratedRooms[RoomIdx].WallSpawnPoints.Num());
    }

    // Generate corridor walls
    for (const FVector& FloorPos : CorridorFloorLocations)
    {
        AddWallsAroundTile(FloorPos, CorridorWalls);
    }

    UE_LOG(DungeonGenerator, Log, TEXT("Generated %d corridor wall spawn points"), CorridorWalls.Num());
}

// Helper function to check if a wall exists anywhere in the dungeon
bool ADungeonGenerator::WallExistsAnywhere(const FVector& WallPos, bool bFacingX) const
{
    float Epsilon = FloorTileSize * 0.05f;

    // Check in all room walls
    for (const FRoom& Room : GeneratedRooms)
    {
        for (const FWallSpawnPoint& Wall : Room.WallSpawnPoints)
        {
            if (FVector::Distance(Wall.WorldLocation, WallPos) < Epsilon && Wall.bFacingX == bFacingX)
            {
                return true;
            }
        }
    }

    // Check in corridor walls
    for (const FWallSpawnPoint& Wall : CorridorWalls)
    {
        if (FVector::Distance(Wall.WorldLocation, WallPos) < Epsilon && Wall.bFacingX == bFacingX)
        {
            return true;
        }
    }

    return false;
}

// Helper function to prevent duplicate wall generation
void ADungeonGenerator::AddWallsAroundTile(const FVector& FloorPos, TArray<FWallSpawnPoint>& OutWalls)
{
    FVector CenterPos = FloorPos;
    float Epsilon = FloorTileSize * 0.05f; // Use 5% of tile size for better precision

    // Check all four directions
    FVector Directions[4] = {
        FVector(-FloorTileSize, 0, 0),  // North
        FVector(0, FloorTileSize, 0),   // East
        FVector(FloorTileSize, 0, 0),   // South
        FVector(0, -FloorTileSize, 0)   // West
    };

    bool FacingX[4] = { true, false, true, false };

    // For each direction, check if we need a wall
    for (int32 i = 0; i < 4; i++)
    {
        if (!HasFloorTileAt(CenterPos + Directions[i]))
        {
            // Calculate wall position (halfway between tiles)
            FVector WallPos = CenterPos + (Directions[i] * 0.5f);

            // Check if this wall position already exists anywhere in the dungeon
            if (!WallExistsAnywhere(WallPos, FacingX[i]))
            {
                OutWalls.Add(FWallSpawnPoint(WallPos, FacingX[i]));
            }
        }
    }
}

// Helper function to check if a floor tile exists at a location
bool ADungeonGenerator::HasFloorTileAt(const FVector& Location) const
{
    float Epsilon = FloorTileSize * 0.05f; // Use 5% of tile size for consistent precision

    // Check room floor tiles
    for (const FRoom& Room : GeneratedRooms)
    {
        for (const FVector& FloorPos : Room.FloorTileWorldLocations)
        {
            if (FVector::Distance(FloorPos, Location) < Epsilon)
            {
                return true;
            }
        }
    }

    // Check corridor floor tiles
    for (const FVector& FloorPos : CorridorFloorLocations)
    {
        if (FVector::Distance(FloorPos, Location) < Epsilon)
        {
            return true;
        }
    }

    return false;
}

void ADungeonGenerator::ConnectRooms()
{
    // Clear any existing corridor data
    CorridorFloorLocations.Empty();
    CorridorCorners.Empty();

    // If there are fewer than 2 rooms, we don't need corridors
    if (GeneratedRooms.Num() < 2)
    {
        UE_LOG(DungeonGenerator, Log, TEXT("Not enough rooms to create corridors"));
        return;
    }

    UE_LOG(DungeonGenerator, Log, TEXT("Connecting %d rooms with corridors"), GeneratedRooms.Num());

    // Connect each room to the next one
    for (int32 i = 0; i < GeneratedRooms.Num() - 1; i++)
    {
        // Find closest floor tiles between this room and the next
        FVector StartPoint = FindClosestFloorTile(GeneratedRooms[i], GeneratedRooms[i + 1]);
        FVector EndPoint = FindClosestFloorTile(GeneratedRooms[i + 1], GeneratedRooms[i]);

        // Generate corridor between them using L-shaped path
        GenerateCorridor(StartPoint, EndPoint);

        UE_LOG(DungeonGenerator, Log, TEXT("Connected Room %d to Room %d"), i, i + 1);
    }

    // Connect first and last room to make a loop (optional)
    if (GeneratedRooms.Num() >= 3)
    {
        FVector StartPoint = FindClosestFloorTile(GeneratedRooms.Last(), GeneratedRooms[0]);
        FVector EndPoint = FindClosestFloorTile(GeneratedRooms[0], GeneratedRooms.Last());

        GenerateCorridor(StartPoint, EndPoint);

        UE_LOG(DungeonGenerator, Log, TEXT("Connected last room to first room to form loop"));
    }

    UE_LOG(DungeonGenerator, Log, TEXT("Created %d corridor floor tiles"), CorridorFloorLocations.Num());
}

// Helper function to find closest floor tiles between rooms
FVector ADungeonGenerator::FindClosestFloorTile(const FRoom& FromRoom, const FRoom& ToRoom)
{
    float MinDistance = MAX_flt;
    FVector ClosestTile = FVector::ZeroVector;

    for (const FVector& FromTile : FromRoom.FloorTileWorldLocations)
    {
        for (const FVector& ToTile : ToRoom.FloorTileWorldLocations)
        {
            float Distance = FVector::Dist(FromTile, ToTile);
            if (Distance < MinDistance)
            {
                MinDistance = Distance;
                ClosestTile = FromTile;
            }
        }
    }

    return ClosestTile;
}

// Helper to generate an L-shaped corridor
void ADungeonGenerator::GenerateCorridor(const FVector& Start, const FVector& End)
{
    // First, add the starting tile
    CorridorFloorLocations.Add(Start);

    // Determine if we should go X then Y, or Y then X
    bool bGoXFirst = FMath::RandBool();

    FVector Current = Start;

    if (bGoXFirst)
    {
        // Move along X axis first
        float StepX = FloorTileSize * (End.X > Start.X ? 1 : -1);

        while (FMath::Abs(Current.X - End.X) > FloorTileSize * 0.5f)
        {
            Current.X += StepX;
            CorridorFloorLocations.Add(Current);
        }

        // Then move along Y axis
        float StepY = FloorTileSize * (End.Y > Current.Y ? 1 : -1);

        while (FMath::Abs(Current.Y - End.Y) > FloorTileSize * 0.5f)
        {
            Current.Y += StepY;
            CorridorFloorLocations.Add(Current);
        }
    }
    else
    {
        // Move along Y axis first
        float StepY = FloorTileSize * (End.Y > Start.Y ? 1 : -1);

        while (FMath::Abs(Current.Y - End.Y) > FloorTileSize * 0.5f)
        {
            Current.Y += StepY;
            CorridorFloorLocations.Add(Current);
        }

        // Then move along X axis
        float StepX = FloorTileSize * (End.X > Current.X ? 1 : -1);

        while (FMath::Abs(Current.X - End.X) > FloorTileSize * 0.5f)
        {
            Current.X += StepX;
            CorridorFloorLocations.Add(Current);
        }
    }
}

void ADungeonGenerator::DetectCorners()
{
    UE_LOG(DungeonGenerator, Log, TEXT("Starting corner detection with threshold=%f"), FloorTileSize * 0.3f);

    // Clear any existing corner data
    CorridorCorners.Empty();
    for (int32 i = 0; i < GeneratedRooms.Num(); i++)
    {
        GeneratedRooms[i].CornerPoints.Empty();
    }

    // Set appropriate distance threshold for corner detection
    float CornerThreshold = FloorTileSize * 0.3f; // Use 30% of floor tile size
    int32 TotalCorners = 0;

    // First pass: identify potential corner locations in rooms
    for (int32 RoomIdx = 0; RoomIdx < GeneratedRooms.Num(); RoomIdx++)
    {
        TArray<FWallSpawnPoint> RoomHorizontalWalls;
        TArray<FWallSpawnPoint> RoomVerticalWalls;

        // Separate walls by orientation
        for (const FWallSpawnPoint& Wall : GeneratedRooms[RoomIdx].WallSpawnPoints)
        {
            if (Wall.bFacingX)
                RoomHorizontalWalls.Add(Wall);
            else
                RoomVerticalWalls.Add(Wall);
        }

        // Log wall counts for debugging
        UE_LOG(DungeonGenerator, Log, TEXT("Room %d has %d horizontal walls, %d vertical walls"),
            RoomIdx, RoomHorizontalWalls.Num(), RoomVerticalWalls.Num());

        // Find intersections between horizontal and vertical walls
        for (const FWallSpawnPoint& HWall : RoomHorizontalWalls)
        {
            for (const FWallSpawnPoint& VWall : RoomVerticalWalls)
            {
                // Calculate distance between walls
                float Distance = FVector::Dist2D(HWall.WorldLocation, VWall.WorldLocation);

                // If walls are close enough, this is a corner
                if (Distance < CornerThreshold)
                {
                    // Calculate corner position (average of the two wall positions)
                    FVector CornerPos = (HWall.WorldLocation + VWall.WorldLocation) * 0.5f;

                    // Add to room corners
                    GeneratedRooms[RoomIdx].CornerPoints.Add(FCornerPoint(CornerPos));
                    TotalCorners++;
                }
            }
        }
    }

    // Second pass: identify corridor corners
    TArray<FWallSpawnPoint> CorridorHorizontalWalls;
    TArray<FWallSpawnPoint> CorridorVerticalWalls;

    // Separate walls by orientation
    for (const FWallSpawnPoint& Wall : CorridorWalls)
    {
        if (Wall.bFacingX)
            CorridorHorizontalWalls.Add(Wall);
        else
            CorridorVerticalWalls.Add(Wall);
    }

    UE_LOG(DungeonGenerator, Log, TEXT("Corridors have %d horizontal walls, %d vertical walls"),
        CorridorHorizontalWalls.Num(), CorridorVerticalWalls.Num());

    // Find intersections between horizontal and vertical walls
    for (const FWallSpawnPoint& HWall : CorridorHorizontalWalls)
    {
        for (const FWallSpawnPoint& VWall : CorridorVerticalWalls)
        {
            // Calculate distance between walls
            float Distance = FVector::Dist2D(HWall.WorldLocation, VWall.WorldLocation);

            // If walls are close enough, this is a corner
            if (Distance < CornerThreshold)
            {
                // Calculate corner position (average of the two wall positions)
                FVector CornerPos = (HWall.WorldLocation + VWall.WorldLocation) * 0.5f;

                // Add to corridor corners
                CorridorCorners.Add(FCornerPoint(CornerPos));
                TotalCorners++;
            }
        }
    }

    // Log accurate total corner count
    int32 RoomCornerCount = 0;
    for (const FRoom& Room : GeneratedRooms)
    {
        RoomCornerCount += Room.CornerPoints.Num();
    }

    UE_LOG(DungeonGenerator, Log, TEXT("Detected %d room corners and %d corridor corners (Total: %d)"),
        RoomCornerCount, CorridorCorners.Num(), TotalCorners);
}

void ADungeonGenerator::SpawnPillars() //Kept only for backward compatability (does not get called)
{
    // Skip if no pillar mesh is assigned
    if (!PillarSettings.PillarMesh)
    {
        UE_LOG(DungeonGenerator, Warning, TEXT("No pillar mesh assigned! Cannot spawn pillars."));
        return;
    }

    // Process all corners from rooms and corridors
    TArray<FCornerPoint> AllCorners;

    // Add room corners
    for (const FRoom& Room : GeneratedRooms)
    {
        AllCorners.Append(Room.CornerPoints);
    }

    // Add corridor corners
    AllCorners.Append(CorridorCorners);

    UE_LOG(DungeonGenerator, Log, TEXT("Attempting to spawn pillars at %d corner locations with chance %f"),
        AllCorners.Num(), PillarSettings.SpawnChance);

    int32 SpawnedPillars = 0;

    // Spawn pillars at corners based on chance
    for (const FCornerPoint& Corner : AllCorners)
    {
        if (FMath::FRand() <= PillarSettings.SpawnChance)
        {
            FVector PillarLocation = Corner.WorldLocation + PillarSettings.LocationOffset;
            FTransform PillarTransform(PillarSettings.RotationOffset, PillarLocation, PillarSettings.Scale);

            AStaticMeshActor* PillarActor = SpawnDungeonMesh(
                PillarTransform,
                PillarSettings.PillarMesh,
                PillarSettings.MaterialOverride
            );

            if (PillarActor)
            {
                PillarActor->Tags.Add(PILLAR_MESH_TAG);
                SpawnedPillars++;
            }
        }
    }

    UE_LOG(DungeonGenerator, Log, TEXT("Successfully spawned %d pillars"), SpawnedPillars);
}

void ADungeonGenerator::SpawnCeilingMeshes(const TArray<FVector>& FloorLocations)
{
    // Skip if no ceiling mesh is assigned
    if (!CeilingSM)
    {
        return;
    }

    // Spawn a ceiling tile above each floor tile
    for (const FVector& FloorPos : FloorLocations)
    {
        FVector CeilingPos = FloorPos + CeilingPivotOffset;
        FTransform CeilingTransform(FRotator::ZeroRotator, CeilingPos);

        AStaticMeshActor* CeilingActor = SpawnDungeonMesh(CeilingTransform, CeilingSM);

        if (CeilingActor)
        {
            CeilingActor->Tags.Add(CEILING_MESH_TAG);
        }
    }
}

void ADungeonGenerator::SpawnFloorDecals(const TArray<FVector>& FloorLocations, bool bIsRoom)
{
    // Skip if no decals are defined
    if (FloorDecalsToSpawn.Num() == 0)
    {
        return;
    }

    // Attempt to spawn decals on floor tiles
    for (const FVector& FloorPos : FloorLocations)
    {
        for (const FDecalSpawnParams& DecalParams : FloorDecalsToSpawn)
        {
            // Skip if this is a corridor tile and the decal should only spawn in rooms
            if (!bIsRoom && DecalParams.bOnlySpawnInRooms)
            {
                continue;
            }

            if (DecalParams.DecalMaterial && FMath::FRand() <= DecalParams.SpawnChance)
            {
                // Create decal component attached to floor actor
                UDecalComponent* DecalComponent = NewObject<UDecalComponent>(this);
                if (DecalComponent)
                {
                    DecalComponent->RegisterComponent();
                    DecalComponent->SetRelativeLocation(FloorPos + FVector(0, 0, DecalParams.ZOffset));

                    // Apply random rotation if specified
                    if (DecalParams.RandomRotationRange > 0)
                    {
                        float RandomRotation = FMath::FRandRange(0, DecalParams.RandomRotationRange);
                        DecalComponent->SetRelativeRotation(FRotator(90.0f, RandomRotation, 0));
                    }
                    else
                    {
                        DecalComponent->SetRelativeRotation(FRotator(90.0f, 0, 0)); // Default orientation for floor decals
                    }

                    DecalComponent->SetDecalMaterial(DecalParams.DecalMaterial);
                    DecalComponent->DecalSize = DecalParams.DecalSize;
                    DecalComponent->SetVisibility(true);
                }

                // Only spawn one decal per location
                break;
            }
        }
    }
}

void ADungeonGenerator::SpawnBlueprintActors(const TArray<FVector>& FloorLocations, bool bIsRoom)
{
    // Skip if no blueprints to spawn
    if (BlueprintActorsToSpawn.Num() == 0)
        return;

    UWorld* World = GetWorld();
    if (!World)
        return;

    int32 SpawnedCount = 0;

    // Try to spawn blueprints at each floor location
    for (const FVector& FloorPos : FloorLocations) // Loop for FloorPos
    {
        for (const FBlueprintSpawnParams& BPParams : BlueprintActorsToSpawn) // Loop for BPParams
        {
            // Skip if this is a corridor and we only want rooms
            if (BPParams.bOnlySpawnInRooms && !bIsRoom)
                continue;

            // Check spawn chance AND if BlueprintClass is valid before attempting to spawn
            if (BPParams.BlueprintClass && FMath::FRand() <= BPParams.SpawnChance)
            {
                // Calculate random offset within min/max range
                FVector RandomOffset;
                RandomOffset.X = FMath::RandRange(BPParams.LocationOffsetMin.X, BPParams.LocationOffsetMax.X);
                RandomOffset.Y = FMath::RandRange(BPParams.LocationOffsetMin.Y, BPParams.LocationOffsetMax.Y);
                RandomOffset.Z = FMath::RandRange(BPParams.LocationOffsetMin.Z, BPParams.LocationOffsetMax.Z);

                // Calculate final spawn position
                FVector SpawnLocation = FloorPos + RandomOffset;

                // Setup spawn parameters
                FActorSpawnParameters SpawnParams;
                SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

                // Spawn the actor
                AActor* NewActor = World->SpawnActor<AActor>( // NewActor is declared here
                    BPParams.BlueprintClass,
                    SpawnLocation,
                    BPParams.RotationOffset,
                    SpawnParams
                );

                // Apply scale and tag for cleanup if spawn was successful
                if (NewActor) // NewActor is valid in this scope
                {
                    NewActor->SetActorScale3D(BPParams.Scale);
                    NewActor->Tags.AddUnique(DUNGEON_BP_TAG);

                    // Corrected Logging Here:
                    UE_LOG(LogTemp, Warning, TEXT("SpawnBlueprintActors: Actor %s spawned. Attempting to tag with %s. Current tags: [%s]"),
                        *NewActor->GetName(),
                        *DUNGEON_BP_TAG.ToString(),
                        *FString::JoinBy(NewActor->Tags, TEXT(", "), [](const FName& Tag) { return Tag.ToString(); }));

                    if (NewActor->Tags.Contains(DUNGEON_BP_TAG)) {
                        UE_LOG(LogTemp, Warning, TEXT("SpawnBlueprintActors: Actor %s successfully CONTAINS tag %s"), *NewActor->GetName(), *DUNGEON_BP_TAG.ToString());
                    }
                    else {
                        UE_LOG(LogTemp, Error, TEXT("SpawnBlueprintActors: Actor %s FAILED to contain tag %s after adding!"), *NewActor->GetName(), *DUNGEON_BP_TAG.ToString());
                    }
                    SpawnedCount++;
                }
                else // NewActor is NULL, but BPParams.BlueprintClass was valid
                {
                    // Corrected Logging Here:
                    UE_LOG(LogTemp, Error, TEXT("SpawnBlueprintActors: Failed to spawn actor for BlueprintClass %s at location %s. SpawnActor returned null."),
                        *BPParams.BlueprintClass->GetName(), // BPParams is in scope
                        *SpawnLocation.ToString());
                }
            }
            else if (!BPParams.BlueprintClass)
            {
                UE_LOG(DungeonGenerator, Error, TEXT("SpawnBlueprintActors: BlueprintClass is null for an entry in BlueprintActorsToSpawn. Skipping spawn."));
            }
        }
    }
    UE_LOG(LogTemp, Log, TEXT("Spawned %d blueprint actors"), SpawnedCount);
}

void ADungeonGenerator::SpawnStaticMeshes(const TArray<FVector>& FloorLocations, bool bIsRoom)
{
    if (StaticMeshesToSpawn.Num() == 0)
        return;

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(DungeonGenerator, Error, TEXT("SpawnStaticMeshes: World is null. Cannot spawn static meshes."));
        return;
    }

    int32 SpawnedMeshes = 0; // Add counter for logging

    for (const FVector& FloorPos : FloorLocations)
    {
        for (const FStaticMeshSpawnParams& MeshParams : StaticMeshesToSpawn)
        {
            // Skip if we only want to spawn in rooms and we're not in a room
            if (MeshParams.bOnlySpawnInRooms && !bIsRoom)
                continue;

            if (!MeshParams.StaticMesh)
            {
                UE_LOG(DungeonGenerator, Error, TEXT("SpawnStaticMeshes: StaticMesh is null for an entry in StaticMeshesToSpawn. Skipping spawn."));
                continue;
            }

            if (FMath::FRand() <= MeshParams.SpawnChance)
            {
                // Create a random offset between min and max values
                FVector RandomOffset;
                RandomOffset.X = FMath::RandRange(MeshParams.LocationOffsetMin.X, MeshParams.LocationOffsetMax.X);
                RandomOffset.Y = FMath::RandRange(MeshParams.LocationOffsetMin.Y, MeshParams.LocationOffsetMax.Y);
                RandomOffset.Z = FMath::RandRange(MeshParams.LocationOffsetMin.Z, MeshParams.LocationOffsetMax.Z);

                FVector SpawnLocation = FloorPos + RandomOffset;

                // Spawn the mesh
                AStaticMeshActor* Mesh = SpawnDungeonMesh(
                    FTransform(MeshParams.RotationOffset, SpawnLocation, MeshParams.Scale),
                    MeshParams.StaticMesh,
                    MeshParams.MaterialOverride
                );

                if (Mesh)
                {
                    SpawnedMeshes++;
                }
            }
        }
    }

    UE_LOG(DungeonGenerator, Log, TEXT("Spawned %d static meshes"), SpawnedMeshes);
}

AStaticMeshActor* ADungeonGenerator::SpawnDungeonMesh(const FTransform& InTransform, UStaticMesh* SMToSpawn, UMaterialInterface* OverrideMaterial)
{
    // Ensure we have a mesh to spawn
    if (!SMToSpawn)
    {
        return nullptr;
    }

    FActorSpawnParameters ActorSpawnParams;
    ActorSpawnParams.Owner = this;
    ActorSpawnParams.Instigator = GetInstigator();
    ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(DungeonGenerator, Error, TEXT("SpawnDungeonMesh: World is null. Cannot spawn mesh."));
        return nullptr;
    }

    AStaticMeshActor* SMActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), InTransform, ActorSpawnParams);
    if (SMActor)
    {
        // Set mobility to static for better performance, but can be changed for interactive elements
        SMActor->GetStaticMeshComponent()->SetMobility(EComponentMobility::Static);
        SMActor->GetStaticMeshComponent()->SetStaticMesh(SMToSpawn);

        // Apply custom material if provided
        if (OverrideMaterial)
        {
            SMActor->GetStaticMeshComponent()->SetMaterial(0, OverrideMaterial);
        }

        // Tag the mesh for easy reference
        SMActor->Tags.Add(DUNGEON_MESH_TAG);

        // Set replication for multiplayer
        SMActor->SetReplicates(true);
        SMActor->SetReplicateMovement(false); // Static meshes don't need movement replication
    }

    return SMActor;
}

void ADungeonGenerator::DestroyDungeonMeshes()
{
    // Arrays to store actors we need to destroy
    TArray<AActor*> DungeonMeshes;
    TArray<AActor*> CeilingMeshes;
    TArray<AActor*> PillarMeshes;
    TArray<AActor*> PropMeshes;
    TArray<AActor*> BlueprintActors;

    // Find all actors with our tags
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), DUNGEON_MESH_TAG, DungeonMeshes);
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), CEILING_MESH_TAG, CeilingMeshes);
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), PILLAR_MESH_TAG, PillarMeshes);
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), PROP_MESH_TAG, PropMeshes);
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), DUNGEON_BP_TAG, BlueprintActors);

    // Log cleanup for debugging
    UE_LOG(LogTemp, Warning, TEXT("Cleaning up dungeon: found %d blueprint actors to destroy"), BlueprintActors.Num());

    // Destroy all found actors
    for (AActor* Actor : DungeonMeshes)
    {
        if (Actor) Actor->Destroy();
    }

    for (AActor* Actor : CeilingMeshes)
    {
        if (Actor) Actor->Destroy();
    }

    for (AActor* Actor : PillarMeshes)
    {
        if (Actor) Actor->Destroy();
    }

    for (AActor* Actor : PropMeshes)
    {
        if (Actor) Actor->Destroy();
    }

    for (AActor* Actor : BlueprintActors)
    {
        if (Actor)
        {
            UE_LOG(LogTemp, Warning, TEXT("Destroying blueprint actor: %s"), *Actor->GetName());
            Actor->Destroy();
        }
    }

    // Make sure to reset data structures too
    GeneratedRooms.Empty();
    CorridorFloorLocations.Empty();
    CorridorWalls.Empty();
    CorridorCorners.Empty();
}

void ADungeonGenerator::ApplyConfigFromDataAsset()
{
    if (!DungeonConfig)
        return;

    // Copy all settings from data asset
    TileMapRows = DungeonConfig->TileMapRows;
    TileMapColumns = DungeonConfig->TileMapColumns;
    MinRoomSize = DungeonConfig->MinRoomSize;
    MaxRoomSize = DungeonConfig->MaxRoomSize;
    RoomsToGenerate = DungeonConfig->RoomsToGenerate;

    // Floor settings
    FloorSM = DungeonConfig->FloorSM;
    FloorTileSize = DungeonConfig->FloorTileSize;
    bAutoFloorTileSizeGeneration = DungeonConfig->bAutoFloorTileSizeGeneration;
    FloorPivotOffset = DungeonConfig->FloorPivotOffset;

    // Wall settings
    WallSM = DungeonConfig->WallSM;
    WallWidth = DungeonConfig->WallWidth;
    WallSMPivotOffset = DungeonConfig->WallSMPivotOffset;
    bWallFacingX = DungeonConfig->bWallFacingX;

    // Ceiling settings
    CeilingSM = DungeonConfig->CeilingSM;
    CeilingHeight = DungeonConfig->CeilingHeight;
    CeilingPivotOffset = DungeonConfig->CeilingPivotOffset;

    // Copy arrays
    BlueprintActorsToSpawn = DungeonConfig->BlueprintActorsToSpawn;
    StaticMeshesToSpawn = DungeonConfig->StaticMeshesToSpawn;
    FloorDecalsToSpawn = DungeonConfig->FloorDecalsToSpawn;
}

void ADungeonGenerator::SpawnThemedRoom(const FRoom& Room)
{
    // Make sure we have a valid theme
    URoomThemeDataAsset* Theme = Room.RoomTheme;
    if (!Theme)
    {
        UE_LOG(LogTemp, Warning, TEXT("SpawnThemedRoom: Attempted to spawn a themed room but Room.RoomTheme was null. Skipping."));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("SpawnThemedRoom: Spawning themed room with theme '%s' for %d floor tiles."), *Theme->GetName(), Room.FloorTileWorldLocations.Num());

    // Spawn floor tiles using theme floor mesh
    for (const FVector& FloorPos : Room.FloorTileWorldLocations)
    {
        SpawnDungeonMesh(
            FTransform(FRotator::ZeroRotator, FloorPos + Theme->FloorPivotOffset),
            Theme->FloorMesh ? Theme->FloorMesh : FloorSM, // Fallback to default if theme mesh is null
            Theme->FloorMaterialOverride
        );

        // Spawn ceiling if available in the theme
        if (Theme->CeilingMesh)
        {
            FVector CeilingPos = FloorPos + Theme->CeilingPivotOffset;
            SpawnDungeonMesh(
                FTransform(FRotator::ZeroRotator, CeilingPos),
                Theme->CeilingMesh,
                Theme->CeilingMaterialOverride
            );
        }
        // Optional: consider if a default ceiling (CeilingSM) should be spawned if Theme->CeilingMesh is null but CeilingSM is valid
    }

    // Spawn walls using theme wall mesh
    for (const FWallSpawnPoint& Wall : Room.WallSpawnPoints)
    {
        FVector WallModifiedOffset = FVector::ZeroVector;
        FRotator WallRotation = CalculateWallRotation(
            Theme->bWallFacingX,
            Wall,
            Theme->WallPivotOffset,
            WallModifiedOffset
        );

        FVector WallSpawnPoint = Wall.WorldLocation + WallModifiedOffset;
        SpawnDungeonMesh(
            FTransform(WallRotation, WallSpawnPoint),
            Theme->WallMesh ? Theme->WallMesh : WallSM, // Fallback to default if theme mesh is null
            Theme->WallMaterialOverride
        );
    }

    // Spawn grid-based meshes (e.g., pits in a specific pattern)
    if (Theme->GridPlacedMeshes.Num() > 0)
    {
        for (const FGridPlacement& GridMeshParams : Theme->GridPlacedMeshes)
        {
            if (!GridMeshParams.Mesh)
            {
                UE_LOG(LogTemp, Warning, TEXT("SpawnThemedRoom: GridPlacedMesh entry has a null Mesh. Skipping this entry."));
                continue;
            }
            if (GridMeshParams.GridPattern.Num() == 0 || GridMeshParams.GridWidth <= 0)
            {
                UE_LOG(LogTemp, Warning, TEXT("SpawnThemedRoom: GridPlacedMesh '%s' has invalid GridPattern or GridWidth. Skipping."), *GridMeshParams.Mesh->GetName());
                continue;
            }

            int32 GridHeight = FMath::CeilToInt(static_cast<float>(GridMeshParams.GridPattern.Num()) / GridMeshParams.GridWidth);
            if (GridHeight <= 0)
            {
                UE_LOG(LogTemp, Warning, TEXT("SpawnThemedRoom: GridPlacedMesh '%s' resulted in invalid GridHeight. Skipping."), *GridMeshParams.Mesh->GetName());
                continue;
            }

            for (int32 x = 0; x < Room.RoomWidth; x++)
            {
                for (int32 y = 0; y < Room.RoomHeight; y++)
                {
                    int32 PatternX = x % GridMeshParams.GridWidth;
                    int32 PatternY = y % GridHeight;
                    int32 Index = PatternY * GridMeshParams.GridWidth + PatternX;

                    if (Index < GridMeshParams.GridPattern.Num() && GridMeshParams.GridPattern[Index] == 1)
                    {
                        FVector WorldPos(
                            (Room.MinGridX + x) * FloorTileSize,
                            (Room.MinGridY + y) * FloorTileSize,
                            0.0f
                        );
                        SpawnDungeonMesh(
                            FTransform(GridMeshParams.Rotation, WorldPos + GridMeshParams.LocationOffset, GridMeshParams.Scale),
                            GridMeshParams.Mesh,
                            GridMeshParams.MaterialOverride
                        );
                    }
                }
            }
        }
    }

    // Spawn theme-specific static meshes
    for (const FVector& FloorPos : Room.FloorTileWorldLocations)
    {
        for (const FStaticMeshSpawnParams& MeshParams : Theme->ThemeSpecificMeshes)
        {
            if (!MeshParams.StaticMesh)
            {
                // UE_LOG(LogTemp, Warning, TEXT("SpawnThemedRoom: A ThemeSpecificMesh entry has a null StaticMesh. Skipping.")); // Optional: log this if needed
                continue;
            }
            if (FMath::FRand() <= MeshParams.SpawnChance)
            {
                FVector RandomOffset;
                RandomOffset.X = FMath::RandRange(MeshParams.LocationOffsetMin.X, MeshParams.LocationOffsetMax.X);
                RandomOffset.Y = FMath::RandRange(MeshParams.LocationOffsetMin.Y, MeshParams.LocationOffsetMax.Y);
                RandomOffset.Z = FMath::RandRange(MeshParams.LocationOffsetMin.Z, MeshParams.LocationOffsetMax.Z);
                FVector SpawnLocation = FloorPos + RandomOffset;

                SpawnDungeonMesh(
                    FTransform(MeshParams.RotationOffset, SpawnLocation, MeshParams.Scale),
                    MeshParams.StaticMesh,
                    MeshParams.MaterialOverride
                );
            }
        }
    }

    // Spawn theme-specific blueprints
    UWorld* World = GetWorld();
    if (World)
    {
        for (const FVector& FloorPos : Room.FloorTileWorldLocations)
        {
            for (const FBlueprintSpawnParams& BPParams : Theme->ThemeSpecificBlueprints)
            {
                // Check spawn chance first
                if (FMath::FRand() <= BPParams.SpawnChance)
                {
                    // Then check if BlueprintClass is valid
                    if (BPParams.BlueprintClass)
                    {
                        FVector RandomOffset;
                        RandomOffset.X = FMath::RandRange(BPParams.LocationOffsetMin.X, BPParams.LocationOffsetMax.X);
                        RandomOffset.Y = FMath::RandRange(BPParams.LocationOffsetMin.Y, BPParams.LocationOffsetMax.Y);
                        RandomOffset.Z = FMath::RandRange(BPParams.LocationOffsetMin.Z, BPParams.LocationOffsetMax.Z);
                        FVector SpawnLocation = FloorPos + RandomOffset;

                        FActorSpawnParameters SpawnInfo;
                        SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

                        AActor* NewActor = World->SpawnActor<AActor>(
                            BPParams.BlueprintClass,
                            SpawnLocation,
                            BPParams.RotationOffset,
                            SpawnInfo
                        );

                        if (NewActor)
                        {
                            NewActor->SetActorScale3D(BPParams.Scale);
                            NewActor->Tags.AddUnique(DUNGEON_BP_TAG);

                            // Logging for successful spawn and tagging
                            UE_LOG(LogTemp, Warning, TEXT("SpawnThemedRoom: Actor %s spawned (theme BP: %s). Attempting to tag with %s. Current tags: [%s]"),
                                *NewActor->GetName(),
                                *BPParams.BlueprintClass->GetName(),
                                *DUNGEON_BP_TAG.ToString(),
                                *FString::JoinBy(NewActor->Tags, TEXT(", "), [](const FName& Tag) { return Tag.ToString(); }));

                            if (NewActor->Tags.Contains(DUNGEON_BP_TAG))
                            {
                                UE_LOG(LogTemp, Warning, TEXT("SpawnThemedRoom: Actor %s (theme BP) successfully CONTAINS tag %s."), *NewActor->GetName(), *DUNGEON_BP_TAG.ToString());
                            }
                            else
                            {
                                UE_LOG(LogTemp, Error, TEXT("SpawnThemedRoom: Actor %s (theme BP) FAILED to contain tag %s after AddUnique! This is unexpected. Tags: [%s]"),
                                    *NewActor->GetName(),
                                    *DUNGEON_BP_TAG.ToString(),
                                    *FString::JoinBy(NewActor->Tags, TEXT(", "), [](const FName& Tag) { return Tag.ToString(); }));
                            }
                        }
                        else
                        {
                            // Logging for failed spawn (SpawnActor returned null)
                            UE_LOG(LogTemp, Error, TEXT("SpawnThemedRoom: Failed to spawn theme actor for BlueprintClass %s at location %s. SpawnActor returned null."),
                                *BPParams.BlueprintClass->GetName(),
                                *SpawnLocation.ToString());
                        }
                    }
                    else
                    {
                        // Logging for null BlueprintClass when spawn chance was met
                        UE_LOG(LogTemp, Warning, TEXT("SpawnThemedRoom: Attempted to spawn theme BP but BlueprintClass was null for an entry in ThemeSpecificBlueprints. SpawnChance (%f) was met."), BPParams.SpawnChance);
                    }
                }
                // No 'else' here if spawn chance wasn't met, as that's expected behavior.
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("SpawnThemedRoom: GetWorld() returned null. Cannot spawn theme-specific blueprints."));
    }
}

void ADungeonGenerator::SpawnDungeonFromDataTable()
{
    // Implementation for spawning from data table
    // This would create rooms based on templates defined in the data table

    if (!RoomTemplatesDataTable)
    {
        UE_LOG(DungeonGenerator, Warning, TEXT("No room templates data table assigned. Using generic dungeon generation instead."));
        return;
    }

    // Get all room templates from the data table
    TArray<FRoomTemplate*> RoomTemplates;
    FString ContextStr;
    RoomTemplatesDataTable->GetAllRows<FRoomTemplate>(ContextStr, RoomTemplates);

    if (RoomTemplates.Num() == 0)
    {
        UE_LOG(DungeonGenerator, Warning, TEXT("Room templates data table is empty. Using generic dungeon generation instead."));
        return;
    }

    // Calculate tile size from first template if needed
    float DataTableFloorTileSize = bAutoFloorTileSizeGeneration ?
        CalculateFloorTileSize(*RoomTemplates[0]->RoomTileMesh) :
        FloorTileSize;

    UE_LOG(DungeonGenerator, Log, TEXT("Spawning dungeon from data table with %d room templates"), RoomTemplates.Num());

    // Spawn rooms using templates
    for (int32 i = 0; i < GeneratedRooms.Num(); i++)
    {
        // Check if this room has a special theme
        if (GeneratedRooms[i].RoomTheme)
        {
            // Use theme-specific meshes for floor, wall, ceiling
            SpawnThemedRoom(GeneratedRooms[i]);
        }
        else
        {
            // Pick a random template for this room
            FRoomTemplate RoomTemplate = *RoomTemplates[FMath::RandRange(0, RoomTemplates.Num() - 1)];

            // Spawn floor tiles for this room
            for (int32 j = 0; j < GeneratedRooms[i].FloorTileWorldLocations.Num(); j++)
            {
                FVector WorldSpawnLocation = GeneratedRooms[i].FloorTileWorldLocations[j];
                SpawnDungeonMesh(
                    FTransform(FRotator::ZeroRotator, WorldSpawnLocation + RoomTemplate.RoomTilePivotOffset),
                    RoomTemplate.RoomTileMesh,
                    RoomTemplate.RoomTileMeshMaterialOverride
                );

                // Spawn ceiling if available
                if (RoomTemplate.CeilingMesh)
                {
                    FVector CeilingPos = WorldSpawnLocation + RoomTemplate.CeilingPivotOffset;
                    SpawnDungeonMesh(
                        FTransform(FRotator::ZeroRotator, CeilingPos),
                        RoomTemplate.CeilingMesh,
                        RoomTemplate.CeilingMaterialOverride
                    );
                }
            }

            // Spawn decals on floor tiles
            SpawnFloorDecals(GeneratedRooms[i].FloorTileWorldLocations, true);

            // Spawn blueprint actors in room
            SpawnBlueprintActors(GeneratedRooms[i].FloorTileWorldLocations, true);

            // Spawn wall tiles for this room
            for (int32 j = 0; j < GeneratedRooms[i].WallSpawnPoints.Num(); j++)
            {
                FVector WallModifiedOffset = FVector::ZeroVector;
                FRotator WallRotation = CalculateWallRotation(
                    RoomTemplate.bIsWallFacingX,
                    GeneratedRooms[i].WallSpawnPoints[j],
                    RoomTemplate.WallMeshPivotOffset,
                    WallModifiedOffset
                );

                FVector WallSpawnLocation = GeneratedRooms[i].WallSpawnPoints[j].WorldLocation + WallModifiedOffset;
                SpawnDungeonMesh(
                    FTransform(WallRotation, WallSpawnLocation),
                    RoomTemplate.WallMesh,
                    RoomTemplate.WallMeshMaterialOverride
                );
            }
        }
    }

    // Use first template for corridors
    FRoomTemplate CorridorTemplate = *RoomTemplates[0];

    // Spawn corridor floor tiles
    for (const FVector& CorridorFloorPos : CorridorFloorLocations)
    {
        SpawnDungeonMesh(
            FTransform(FRotator::ZeroRotator, CorridorFloorPos + CorridorTemplate.RoomTilePivotOffset),
            CorridorTemplate.RoomTileMesh
        );

        // Spawn ceiling for corridors if available
        if (CorridorTemplate.CeilingMesh)
        {
            FVector CeilingPos = CorridorFloorPos + CorridorTemplate.CeilingPivotOffset;
            SpawnDungeonMesh(
                FTransform(FRotator::ZeroRotator, CeilingPos),
                CorridorTemplate.CeilingMesh
            );
        }
    }

    // Spawn decals on corridor floor tiles
    SpawnFloorDecals(CorridorFloorLocations, false);

    // Spawn blueprint actors in corridors
    SpawnBlueprintActors(CorridorFloorLocations, false);

    // Spawn corridor walls
    bool bCorridorWallFacingX = CorridorTemplate.bIsWallFacingX;
    FVector CorridorWallOffset = CorridorTemplate.WallMeshPivotOffset;

    for (int32 i = 0; i < CorridorWalls.Num(); i++)
    {
        FVector WallModifiedOffset = FVector::ZeroVector;
        FRotator WallRotation = CalculateWallRotation(
            bCorridorWallFacingX,
            CorridorWalls[i],
            CorridorWallOffset,
            WallModifiedOffset
        );

        FVector WallSpawnPoint = CorridorWalls[i].WorldLocation + WallModifiedOffset;
        SpawnDungeonMesh(
            FTransform(WallRotation, WallSpawnPoint),
            CorridorTemplate.WallMesh
        );
    }

    // Create combined array of all floor locations
    TArray<FVector> AllFloorLocations;

    // Add room floor tiles
    for (const FRoom& Room : GeneratedRooms)
    {
        AllFloorLocations.Append(Room.FloorTileWorldLocations);
    }

    // Add corridor floor tiles
    AllFloorLocations.Append(CorridorFloorLocations);

    // Spawn static meshes instead of pillars 
    SpawnStaticMeshes(AllFloorLocations, true);

    // Broadcast that the dungeon spawning is complete
    UE_LOG(DungeonGenerator, Log, TEXT("Dungeon spawning from data table complete"));
    if (OnDungeonSpawned.IsBound())
    {
        OnDungeonSpawned.Broadcast();
    }
}

void ADungeonGenerator::SpawnGenericDungeon(const TArray<FVector>& FloorTileLocations, const TArray<FWallSpawnPoint>& WallSpawnPoints)
{
    // Validate required meshes
    if (!FloorSM)
    {
        UE_LOG(DungeonGenerator, Error, TEXT("Cannot generate dungeon: Invalid FloorSM. Verify you have assigned a valid floor mesh"));
        return;
    }

    if (!WallSM)
    {
        UE_LOG(DungeonGenerator, Error, TEXT("Cannot generate dungeon: Invalid WallSM. Verify you have assigned a valid wall mesh"));
        return;
    }

    UE_LOG(DungeonGenerator, Log, TEXT("Spawning generic dungeon with %d floor tiles and %d wall points"),
        FloorTileLocations.Num(), WallSpawnPoints.Num());

    // Instead of going through all floor tiles at once, process each room first
    // so we can apply special themes
    for (int32 i = 0; i < GeneratedRooms.Num(); i++)
    {
        const FRoom& Room = GeneratedRooms[i];

        // Check if this room has a theme
        if (Room.RoomTheme)
        {
            SpawnThemedRoom(Room);
        }
        else
        {
            // Spawn regular floor/ceiling for this room
            for (const FVector& FloorPos : Room.FloorTileWorldLocations)
            {
                // Draw debug boxes if needed
#if WITH_EDITOR
                if (bDebugActive)
                {
                    DrawDebugBox(
                        GetWorld(),
                        FloorPos,
                        DebugVertexBoxExtents,
                        DefaultFloorSpawnLocationColor.ToFColor(true),
                        true,
                        1555.f,
                        0,
                        10.0f
                    );

                    DrawDebugBox(
                        GetWorld(),
                        FloorPos + FloorPivotOffset,
                        DebugVertexBoxExtents,
                        OffsetedFloorSpawnLocationColor.ToFColor(true),
                        true,
                        1555.f,
                        0,
                        10.0f
                    );
                }
#endif

                // Spawn the floor mesh
                SpawnDungeonMesh(
                    FTransform(FRotator::ZeroRotator, FloorPos + FloorPivotOffset),
                    FloorSM
                );

                // Spawn ceiling if available
                if (CeilingSM)
                {
                    FVector CeilingPos = FloorPos + CeilingPivotOffset;
                    SpawnDungeonMesh(
                        FTransform(FRotator::ZeroRotator, CeilingPos),
                        CeilingSM
                    );
                }
            }

            // Spawn regular walls for this room  
            for (const FWallSpawnPoint& Wall : Room.WallSpawnPoints)
            {
                FVector WallModifiedOffset = FVector::ZeroVector;
                FRotator WallRotation = CalculateWallRotation(
                    bWallFacingX,
                    Wall,
                    WallSMPivotOffset,
                    WallModifiedOffset
                );

                FVector WallSpawnPoint = Wall.WorldLocation + WallModifiedOffset;

                // Draw debug boxes if needed
#if WITH_EDITOR
                if (bDebugActive)
                {
                    DrawDebugBox(
                        GetWorld(),
                        Wall.WorldLocation,
                        DebugVertexBoxExtents,
                        DefaultWallSpawnLocationColor.ToFColor(true),
                        true,
                        1555.f,
                        0,
                        10.0f
                    );

                    DrawDebugBox(
                        GetWorld(),
                        WallSpawnPoint,
                        DebugVertexBoxExtents,
                        OffsetedWallSpawnLocationColor.ToFColor(true),
                        true,
                        1555.f,
                        0,
                        10.0f
                    );
                }
#endif

                // Spawn the wall mesh
                SpawnDungeonMesh(
                    FTransform(WallRotation, WallSpawnPoint),
                    WallSM
                );
            }

            // Spawn floor decals for this room
            SpawnFloorDecals(Room.FloorTileWorldLocations, true);

            // Spawn blueprint actors for this room
            SpawnBlueprintActors(Room.FloorTileWorldLocations, true);

            // Spawn static meshes for this room
            SpawnStaticMeshes(Room.FloorTileWorldLocations, true);
        }
    }

    // Now handle corridors
    for (const FVector& FloorPos : CorridorFloorLocations)
    {
        // Spawn corridor floor
        SpawnDungeonMesh(
            FTransform(FRotator::ZeroRotator, FloorPos + FloorPivotOffset),
            FloorSM
        );

        // Spawn corridor ceiling
        if (CeilingSM)
        {
            FVector CeilingPos = FloorPos + CeilingPivotOffset;
            SpawnDungeonMesh(
                FTransform(FRotator::ZeroRotator, CeilingPos),
                CeilingSM
            );
        }
    }

    // Spawn corridor walls
    for (const FWallSpawnPoint& Wall : CorridorWalls)
    {
        FVector WallModifiedOffset = FVector::ZeroVector;
        FRotator WallRotation = CalculateWallRotation(
            bWallFacingX,
            Wall,
            WallSMPivotOffset,
            WallModifiedOffset
        );

        FVector WallSpawnPoint = Wall.WorldLocation + WallModifiedOffset;
        SpawnDungeonMesh(
            FTransform(WallRotation, WallSpawnPoint),
            WallSM
        );
    }

    // Spawn decals and props in corridors
    SpawnFloorDecals(CorridorFloorLocations, false);
    SpawnBlueprintActors(CorridorFloorLocations, false);
    SpawnStaticMeshes(CorridorFloorLocations, false);

    // Broadcast that the dungeon spawning is complete
    UE_LOG(DungeonGenerator, Log, TEXT("Generic dungeon spawning complete"));
    if (OnDungeonSpawned.IsBound())
    {
        OnDungeonSpawned.Broadcast();
    }
}

void ADungeonGenerator::GenerateDungeon()
{
    if (bUseConfigDataAsset && DungeonConfig)
    {
        ApplyConfigFromDataAsset();
    }

    // Check execution authority in multiplayer (but allow editor generation)
    if (GetNetMode() != NM_Standalone && !HasAuthority() && !GIsEditor)
    {
        UE_LOG(DungeonGenerator, Warning, TEXT("GenerateDungeon called on client. Only the server should generate dungeons in multiplayer."));
        return;
    }

    // FIRST CALL: Clean up the old dungeon
    DestroyDungeonMeshes();

    // Set the seed for consistent generation across server/clients
    if (bUseFixedSeed)
    {
        DungeonSeed = FixedSeed;
    }
    else if (HasAuthority() || GIsEditor) // Server or Editor should set a new random seed
    {
        DungeonSeed = FMath::Rand();
    }

    UE_LOG(DungeonGenerator, Log, TEXT("Generating dungeon with seed: %d"), DungeonSeed);

    // Initialize the tile matrix
    InitTileMatrix();

    // Generate rooms
    GenerateRooms();

    // Connect rooms with corridors
    ConnectRooms();

    // Ensure wall spawn points are correctly generated
    EnsureWallSpawnPoints();

    // Detect corners AFTER walls are generated
    DetectCorners();


    // Spawn the dungeon meshes
    if (RoomTemplatesDataTable)
    {
        SpawnDungeonFromDataTable();
    }
    else
    {
        // Get all floor locations from rooms
        TArray<FVector> AllFloorLocations;
        TArray<FWallSpawnPoint> AllWallPoints;

        // Add room floors
        for (const FRoom& Room : GeneratedRooms)
        {
            AllFloorLocations.Append(Room.FloorTileWorldLocations);
            AllWallPoints.Append(Room.WallSpawnPoints);
        }

        // Add corridor floors
        AllFloorLocations.Append(CorridorFloorLocations);
        AllWallPoints.Append(CorridorWalls);

        // Debug output
        UE_LOG(DungeonGenerator, Log, TEXT("Generated %d floor tiles and %d wall points"),
            AllFloorLocations.Num(), AllWallPoints.Num());

        // Spawn generic dungeon
        SpawnGenericDungeon(AllFloorLocations, AllWallPoints);
    }
}

void ADungeonGenerator::SetNewRoomSize(int32 NewMinRoomSize, int32 NewMaxRoomSize)
{
    MinRoomSize = FMath::Max(2, NewMinRoomSize);
    MaxRoomSize = FMath::Max(MinRoomSize + 1, NewMaxRoomSize);
}

void ADungeonGenerator::SetNewFloorMesh(UStaticMesh* NewFloorMesh, FVector NewFloorPivotOffset, bool bAutoFloorSizeGeneration, float OverrideFloorTileSize)
{
    if (NewFloorMesh)
    {
        FloorSM = NewFloorMesh;
        FloorPivotOffset = NewFloorPivotOffset;

        if (bAutoFloorSizeGeneration)
        {
            FloorTileSize = CalculateFloorTileSize(*FloorSM);
        }
        else
        {
            FloorTileSize = OverrideFloorTileSize > 0.0f ? OverrideFloorTileSize : 400.0f;
        }
    }
}

void ADungeonGenerator::SetNewWallMesh(UStaticMesh* NewWallMesh, FVector NewWallSMPivotOffset, bool bIsWallFacingX)
{
    if (NewWallMesh)
    {
        WallSM = NewWallMesh;
        WallSMPivotOffset = NewWallSMPivotOffset;
        bWallFacingX = bIsWallFacingX;
    }
}

void ADungeonGenerator::SetNewCeilingMesh(UStaticMesh* NewCeilingMesh, FVector NewCeilingPivotOffset, float NewCeilingHeight)
{
    if (NewCeilingMesh)
    {
        CeilingSM = NewCeilingMesh;
        CeilingPivotOffset = NewCeilingPivotOffset;
        CeilingHeight = NewCeilingHeight;
    }
}

#if WITH_EDITOR
void ADungeonGenerator::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    // Auto-calculate floor tile size if requested
    if (FloorSM && bAutoFloorTileSizeGeneration)
    {
        FloorTileSize = CalculateFloorTileSize(*FloorSM);
    }

    // Make sure MinRoomSize is never greater than MaxRoomSize
    if (MinRoomSize > MaxRoomSize)
    {
        MaxRoomSize = MinRoomSize + 1;
    }
}
#endif