// TileMatrix.cpp
// BR_BETRAYAL_2025 - Tile matrix for procedural dungeon generation

#include "TileMatrix.h"

DEFINE_LOG_CATEGORY(TileMatrixLog);

int32 FTileMatrix::ManhattanDistance(const Tile& A, const Tile& B)
{
    return FMath::Abs(A.Get<0>() - B.Get<0>()) + FMath::Abs(A.Get<1>() - B.Get<1>());
}

void FTileMatrix::StoreGeneratedRoom(const FRoomTileCollection& InRoom)
{
    GeneratedRooms.Add(InRoom);
    // Connect to previously generated room (except for first room)
    if (GeneratedRooms.Num() > 1)
    {
        ConnectRooms(InRoom, GeneratedRooms.Last(1));
    }
}

void FTileMatrix::ConnectRooms(const FRoomTileCollection& A, const FRoomTileCollection& B)
{
    TArray<Tile> OccupiedTilesA = A.OccupiedTiles;
    TArray<Tile> OccupiedTilesB = B.OccupiedTiles;

    // Container for all possible connections between tiles
    TArray<FTileConnection> TileConnections;

    // Connect room with more tiles to room with fewer tiles for efficiency
    if (OccupiedTilesB.Num() > OccupiedTilesA.Num())
    {
        for (int32 i = 0; i < OccupiedTilesB.Num(); i++)
        {
            for (int32 j = 0; j < OccupiedTilesA.Num(); j++)
            {
                TileConnections.Add(FTileConnection(OccupiedTilesB[i], OccupiedTilesA[j]));
            }
        }
    }
    else
    {
        for (int32 i = 0; i < OccupiedTilesA.Num(); i++)
        {
            for (int32 j = 0; j < OccupiedTilesB.Num(); j++)
            {
                TileConnections.Add(FTileConnection(OccupiedTilesA[i], OccupiedTilesB[j]));
            }
        }
    }

    // Find shortest route between rooms
    int32 PathLength = MAX_int32;
    FTileConnection Path;

    for (int32 i = 0; i < TileConnections.Num(); i++)
    {
        if (TileConnections[i].Length() < PathLength)
        {
            Path = TileConnections[i];
            PathLength = TileConnections[i].Length();
        }
    }

    // Create corridor between rooms using the shortest path
    Tile PivotTile = Path.Start;

    while (PathLength > 1)
    {
        TArray<Tile> NearbyTiles = GetNearbyTiles(PivotTile);
        int32 TempPath = MAX_int32;
        Tile ClosestTile;
        
        // Get closest tile to target based on the nearby tiles
        for (int32 i = 0; i < NearbyTiles.Num(); i++)
        {
            if (ManhattanDistance(NearbyTiles[i], Path.End) < TempPath)
            {
                TempPath = ManhattanDistance(NearbyTiles[i], Path.End);
                ClosestTile = NearbyTiles[i];
            }
        }

        // Mark tile as part of corridor
        OccupyTile(ClosestTile);
        PivotTile = ClosestTile;
        PathLength = ManhattanDistance(PivotTile, Path.End);
    }
}

FTileMatrix::FTileMatrix()
{
    RowsNum = -1;
    ColumnsNum = -1;
    GeneratedRooms.Empty();
}

FTileMatrix::FTileMatrix(int32 RowCount, int32 ColumnCount)
{
    InitTileMap(RowCount, ColumnCount);
}

TArray<FTileMatrix::Tile> FTileMatrix::GetNearbyTiles(const Tile& InTile) const
{
    Tile UpTile = Tile(InTile.Get<0>() - 1, InTile.Get<1>());
    Tile RightTile = Tile(InTile.Get<0>(), InTile.Get<1>() + 1);
    Tile LeftTile = Tile(InTile.Get<0>(), InTile.Get<1>() - 1);
    Tile DownTile = Tile(InTile.Get<0>() + 1, InTile.Get<1>());

    TArray<Tile> NearbyTiles;
    if (IsTileInMap(UpTile))
    {
        NearbyTiles.Add(UpTile);
    }
    if (IsTileInMap(RightTile))
    {
        NearbyTiles.Add(RightTile);
    }
    if (IsTileInMap(LeftTile))
    {
        NearbyTiles.Add(LeftTile);
    }
    if (IsTileInMap(DownTile))
    {
        NearbyTiles.Add(DownTile);
    }
    return NearbyTiles;
}

void FTileMatrix::InitTileMap(int32 Rows, int32 Columns)
{
    RowsNum = Rows;
    ColumnsNum = Columns;

    TileMap.Empty();
    for (int32 i = 0; i < Rows; i++)
    {
        TArray<bool> SingleRow;
        SingleRow.SetNum(Columns);
        for (int32 j = 0; j < Columns; j++)
        {
            SingleRow[j] = false;
        }
        TileMap.Add(SingleRow);
    }
    
    // Clear generated rooms
    GeneratedRooms.Empty();
}

FString FTileMatrix::TileToString(const Tile& InTile) const
{
    return FString("[") + FString::FromInt(InTile.Get<0>()) + "," + FString::FromInt(InTile.Get<1>()) + FString("]");
}

void FTileMatrix::SetRoomSize(int32 NewMinRoomSize, int32 NewMaxRoomSize)
{
    MinRoomSize = FMath::Max(2, NewMinRoomSize);
    MaxRoomSize = FMath::Max(MinRoomSize + 1, NewMaxRoomSize);
}

FTileMatrix::Tile FTileMatrix::GetRandomTile() const
{
    return Tile(FMath::RandRange(0, RowsNum - 1), FMath::RandRange(0, ColumnsNum - 1));
}

bool FTileMatrix::IsTileInMap(const Tile& InTile) const
{
    return InTile.Get<0>() >= 0 && InTile.Get<0>() < RowsNum && 
           InTile.Get<1>() >= 0 && InTile.Get<1>() < ColumnsNum;
}

bool FTileMatrix::IsTileOccupied(Tile InTile) const
{
    if (IsTileInMap(InTile))
    {
        return TileMap[InTile.Get<0>()][InTile.Get<1>()];
    }
    return false;
}

bool FTileMatrix::AreTilesValid(const TArray<Tile>& InTiles) const
{
    for (int32 i = 0; i < InTiles.Num(); i++)
    {
        if (!IsTileInMap(InTiles[i]) || IsTileOccupied(InTiles[i]))
        {
            return false;
        }
    }
    return true;
}

bool FTileMatrix::GetLeftTile(const Tile& InTile, Tile& LeftTile) const
{
    if (IsTileInMap(InTile))
    {
        LeftTile = Tile(InTile.Get<0>(), InTile.Get<1>() - 1);
        return IsTileInMap(LeftTile);
    }
    return false;
}

bool FTileMatrix::GetRightTile(const Tile& InTile, Tile& RightTile) const
{
    if (IsTileInMap(InTile))
    {
        RightTile = Tile(InTile.Get<0>(), InTile.Get<1>() + 1);
        return IsTileInMap(RightTile);
    }
    return false;
}

bool FTileMatrix::GetUpTile(const Tile& InTile, Tile& UpTile) const
{
    if (IsTileInMap(InTile))
    {
        UpTile = Tile(InTile.Get<0>() - 1, InTile.Get<1>());
        return IsTileInMap(UpTile);
    }
    return false;
}

bool FTileMatrix::GetDownTile(const Tile& InTile, Tile& DownTile) const
{
    if (IsTileInMap(InTile))
    {
        DownTile = Tile(InTile.Get<0>() + 1, InTile.Get<1>());
        return IsTileInMap(DownTile);
    }
    return false;
}

TArray<FTileMatrix::FWallSpawnPoint> FTileMatrix::GenerateWallSpawnPointsFromNearbyTiles(const Tile& CenterTile, float TileSize) const
{
    TArray<FWallSpawnPoint> WallSpawnPoints;

    FVector FloorCenter = FVector(CenterTile.Get<0>() * TileSize, CenterTile.Get<1>() * TileSize, 0);
    
    Tile NearbyTile;

    // Check in each direction (up, right, down, left)
    // Spawn walls where there's no occupied tile
    if ((GetUpTile(CenterTile, NearbyTile) && !IsTileOccupied(NearbyTile))
        || !GetUpTile(CenterTile, NearbyTile))
    {
        FVector WallLocation = FloorCenter - FVector(TileSize / 2.f, 0.f, 0.f);
        WallSpawnPoints.Add(FWallSpawnPoint(WallLocation));
    }
    if ((GetRightTile(CenterTile, NearbyTile) && !IsTileOccupied(NearbyTile))
        || !GetRightTile(CenterTile, NearbyTile))
    {
        FVector WallLocation = FloorCenter + FVector(0.f, TileSize / 2.f, 0.f);
        WallSpawnPoints.Add(FWallSpawnPoint(WallLocation, false));
    }
    if ((GetDownTile(CenterTile, NearbyTile) && !IsTileOccupied(NearbyTile))
        || !GetDownTile(CenterTile, NearbyTile))
    {
        FVector WallLocation = FloorCenter + FVector(TileSize / 2.f, 0.f, 0.f);
        WallSpawnPoints.Add(FWallSpawnPoint(WallLocation));
    }
    if ((GetLeftTile(CenterTile, NearbyTile) && !IsTileOccupied(NearbyTile))
        || !GetLeftTile(CenterTile, NearbyTile))
    {
        FVector WallLocation = FloorCenter - FVector(0.f, TileSize / 2.f, 0.f);
        WallSpawnPoints.Add(FWallSpawnPoint(WallLocation, false));
    }

    return WallSpawnPoints;
}

void FTileMatrix::OccupyTile(const Tile& InTile)
{
    if (IsTileInMap(InTile))
    {
        TileMap[InTile.Get<0>()][InTile.Get<1>()] = true;
    }
}

bool FTileMatrix::CreateUpperRightRoomExpansion(const Tile& StartTile, int32 ExpansionCount, TArray<Tile>& RoomTiles) const
{
    RoomTiles.Empty();
    Tile NewTile = Tile(StartTile.Get<0>() + 1, StartTile.Get<1>());

    for (int32 i = 0; i < ExpansionCount; i++)
    {
        NewTile = Tile(NewTile.Get<0>() - 1, StartTile.Get<1>());
        
        for (int32 j = 0; j < ExpansionCount; j++)
        {
            NewTile = Tile(NewTile.Get<0>(), NewTile.Get<1>() + 1);
            RoomTiles.Add(NewTile);
        }
    }
    return AreTilesValid(RoomTiles);
}

bool FTileMatrix::CreateLowerRightRoomExpansion(const Tile& StartTile, int32 ExpansionCount, TArray<Tile>& RoomTiles) const
{
    RoomTiles.Empty();
    Tile NewTile = Tile(StartTile.Get<0>(), StartTile.Get<1>());

    for (int32 i = 0; i < ExpansionCount; i++)
    {
        NewTile = Tile(NewTile.Get<0>() + 1, StartTile.Get<1>());
        
        for (int32 j = 0; j < ExpansionCount; j++)
        {
            NewTile = Tile(NewTile.Get<0>(), NewTile.Get<1>() + 1);
            RoomTiles.Add(NewTile);
        }
    }
    return AreTilesValid(RoomTiles);
}

bool FTileMatrix::CreateUpperLeftRoomExpansion(const Tile& StartTile, int32 ExpansionCount, TArray<Tile>& RoomTiles) const
{
    RoomTiles.Empty();
    Tile NewTile = Tile(StartTile.Get<0>(), StartTile.Get<1>());

    for (int32 i = 0; i < ExpansionCount; i++)
    {
        NewTile = Tile(NewTile.Get<0>() - 1, StartTile.Get<1>());
        
        for (int32 j = 0; j < ExpansionCount; j++)
        {
            NewTile = Tile(NewTile.Get<0>(), NewTile.Get<1>() - 1);
            RoomTiles.Add(NewTile);
        }
    }
    return AreTilesValid(RoomTiles);
}

bool FTileMatrix::CreateLowerLeftRoomExpansion(const Tile& StartTile, int32 ExpansionCount, TArray<Tile>& RoomTiles) const
{
    RoomTiles.Empty();
    Tile NewTile = Tile(StartTile.Get<0>(), StartTile.Get<1>());

    for (int32 i = 0; i < ExpansionCount; i++)
    {
        NewTile = Tile(NewTile.Get<0>() + 1, StartTile.Get<1>());
        
        for (int32 j = 0; j < ExpansionCount; j++)
        {
            NewTile = Tile(NewTile.Get<0>(), NewTile.Get<1>() - 1);
            RoomTiles.Add(NewTile);
        }
    }
    return AreTilesValid(RoomTiles);
}

bool FTileMatrix::CanPlaceRoomInTileMap(Tile InTile, int32 RoomSize, TArray<Tile>& TilesToOccupy) const
{
    TilesToOccupy.Empty();
    if (!IsTileOccupied(InTile))
    {
        // Try different expansion directions
        if (CreateUpperRightRoomExpansion(InTile, RoomSize, TilesToOccupy))
        {
            return true;
        }
        else if (CreateLowerRightRoomExpansion(InTile, RoomSize, TilesToOccupy))
        {
            return true;
        }
        else if (CreateUpperLeftRoomExpansion(InTile, RoomSize, TilesToOccupy))
        {
            return true;
        }
        else if (CreateLowerLeftRoomExpansion(InTile, RoomSize, TilesToOccupy))
        {
            return true;
        }
    }
    return false;
}

void FTileMatrix::CreateRooms(int32 RoomCount, int32 Seed)
{
    // Initialize random seed
    FMath::RandInit(Seed);
    
    // Clear any existing rooms
    GeneratedRooms.Empty();
    
    for (int32 i = 0; i < RoomCount; i++)
    {
        bool bGeneratedRandomRoom = false;

        for (int32 j = 0; j < MaxRandomAttemptsPerRoom && !bGeneratedRandomRoom; j++)
        {
            int32 RoomSize = FMath::RandRange(MinRoomSize, MaxRoomSize);
            Tile RandomTile = GetRandomTile();
            TArray<Tile> RoomTiles;

            if (CanPlaceRoomInTileMap(RandomTile, RoomSize, RoomTiles))
            {
                // Occupy tiles
                for (int32 k = 0; k < RoomTiles.Num(); k++)
                {
                    OccupyTile(RoomTiles[k]);
                }
                StoreGeneratedRoom(FRoomTileCollection(RoomTiles));
                bGeneratedRandomRoom = true;
            }
        }
    }
}

void FTileMatrix::PrintDebugTileMap() const
{
    UE_LOG(TileMatrixLog, Warning, TEXT(" ---- Printing Debug Tile Map ---- "));
    
    for (int32 i = 0; i < TileMap.Num(); i++)
    {
        FString Row;
        for (int32 j = 0; j < TileMap[i].Num() - 1; j++)
        {
            FString ElementStr = ((TileMap[i])[j]) ? FString("1") : FString("0");
            ElementStr.Append(" - ");
            Row.Append(ElementStr);
        }
        FString ElementStr = ((TileMap[i])[TileMap[i].Num() - 1]) ? FString("1") : FString("0");
        Row.Append(ElementStr);
        
        UE_LOG(TileMatrixLog, Warning, TEXT("%s"), *FString(Row));
    }

    UE_LOG(TileMatrixLog, Warning, TEXT(" ---- End Of Printing Debug Tile Map ----"));
}

void FTileMatrix::ProjectTileMapLocationsToWorld(float TileSize, TArray<FVector>& FloorLocations, TArray<FWallSpawnPoint>& WallLocations)
{
    FloorLocations.Empty();
    WallLocations.Empty();

    // Process all tiles
    for (int32 i = 0; i < TileMap.Num(); i++)
    {
        for (int32 j = 0; j < TileMap[i].Num(); j++)
        {
            Tile CurrentTile = Tile(i, j);
            if (IsTileOccupied(CurrentTile))
            {
                // Add floor location
                FVector FloorCenter = FVector(i * TileSize, j * TileSize, 0);
                FloorLocations.Add(FloorCenter);

                // Add wall locations
                TArray<FWallSpawnPoint> WallSpawnPoints = GenerateWallSpawnPointsFromNearbyTiles(CurrentTile, TileSize);
                WallLocations.Append(WallSpawnPoints);
            }
        }
    }
    
    // Detect corners for pillars
    TArray<FCornerPoint> Corners;
    DetectCorners(TileSize, Corners);
}

void FTileMatrix::ProjectTileMapLocationsToWorld(float TileSize, TArray<FRoom>& Rooms, TArray<FVector>& CorridorFloorTiles, TArray<FWallSpawnPoint>& CorridorWalls)
{
    // Set to keep track of tiles that are part of rooms
    TSet<Tile> RecordedRoomTiles;

    // Clear output arrays
    Rooms.Empty();
    CorridorFloorTiles.Empty();
    CorridorWalls.Empty();
    
    // Reserve space for rooms
    Rooms.Reserve(GeneratedRooms.Num());

    // Process each room
    for (int32 i = 0; i < GeneratedRooms.Num(); i++)
    {
        FRoom NewRoom;
        TArray<Tile> RoomTiles = GeneratedRooms[i].OccupiedTiles;

        // Process each tile in the room
        for (int32 j = 0; j < RoomTiles.Num(); j++)
        {
            Tile CurrentTile = RoomTiles[j];
            RecordedRoomTiles.Add(CurrentTile); // Mark as part of a room

            // Add floor location
            FVector FloorCenter = FVector(CurrentTile.Get<0>() * TileSize, CurrentTile.Get<1>() * TileSize, 0);
            NewRoom.FloorTileWorldLocations.Add(FloorCenter);
            
            // Add wall locations
            TArray<FWallSpawnPoint> RoomWallSpawnPoints = GenerateWallSpawnPointsFromNearbyTiles(CurrentTile, TileSize);
            NewRoom.WallSpawnPoints.Append(RoomWallSpawnPoints);
        }
        
        Rooms.Add(NewRoom);
    }

    // Process corridors (tiles that are occupied but not part of any room)
    for (int32 i = 0; i < TileMap.Num(); i++)
    {
        for (int32 j = 0; j < TileMap[i].Num(); j++)
        {
            Tile CurrentTile = Tile(i, j);
            if (IsTileOccupied(CurrentTile) && !RecordedRoomTiles.Contains(CurrentTile))
            {
                // Add corridor floor tile
                FVector FloorCenter = FVector(i * TileSize, j * TileSize, 0);
                CorridorFloorTiles.Add(FloorCenter);

                // Add corridor wall locations
                TArray<FWallSpawnPoint> WallSpawnPoints = GenerateWallSpawnPointsFromNearbyTiles(CurrentTile, TileSize);
                CorridorWalls.Append(WallSpawnPoints);
            }
        }
    }
    
    // Detect corners for rooms and corridors
    for (FRoom& Room : Rooms)
    {
        TArray<FCornerPoint> RoomCorners;
        DetectCorners(TileSize, RoomCorners);
        Room.CornerPoints = RoomCorners;
    }
}

void FTileMatrix::DetectCorners(float TileSize, TArray<FCornerPoint>& OutCorners)
{
    // Complex corner detection logic goes here
    // This is a placeholder - you would need to implement the actual corner detection
    // based on wall intersection points
    
    // For an initial implementation, we could check where vertical and horizontal walls meet
    // by examining neighboring tiles and wall orientations
    
    OutCorners.Empty();
    
    // Simple corner detection algorithm would go here
    // For now, we just identify some potential corner locations based on adjacent walls
    
    // Example implementation:
    // Iterate through the tile map and look for corner patterns
    // A corner usually means a tile that has two adjacent walls in different orientations
}