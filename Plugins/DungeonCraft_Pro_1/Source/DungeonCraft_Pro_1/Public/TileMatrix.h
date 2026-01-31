// TileMatrix.h
// BR_BETRAYAL_2025 - Tile matrix for procedural dungeon generation

#pragma once

#include "CoreMinimal.h"
#include "TileMatrix.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(TileMatrixLog, Log, All);

// Forward declarations
struct FRoom;

/**
 * TileMatrix - Handles the 2D grid representation and procedural generation logic for dungeons
 */
USTRUCT(BlueprintType)
struct DUNGEONCRAFT_PRO_1_API FTileMatrix
{
    GENERATED_BODY()

public:
    // Wall spawn point definition
    struct FWallSpawnPoint
    {
        FVector WorldLocation;
        bool bFacingX;
        bool bIsCorner;

        FWallSpawnPoint() : WorldLocation(FVector()), bFacingX(true), bIsCorner(false) {}
        FWallSpawnPoint(FVector NewWorldLocation, bool IsFacingX = true) 
            : WorldLocation(NewWorldLocation), bFacingX(IsFacingX), bIsCorner(false) {}
    };

    // Corner point definition for pillar placement
    struct FCornerPoint
    {
        FVector WorldLocation;
        
        FCornerPoint() : WorldLocation(FVector::ZeroVector) {}
        FCornerPoint(FVector InWorldLocation) : WorldLocation(InWorldLocation) {}
    };

    // Room definition
    struct FRoom
    {
        TArray<FVector> FloorTileWorldLocations;
        TArray<FWallSpawnPoint> WallSpawnPoints;
        TArray<FCornerPoint> CornerPoints;
    };

    // Constructors
    FTileMatrix();
    FTileMatrix(int32 RowCount, int32 ColumnCount);

    // Initialize the tile matrix with given dimensions
    void InitTileMap(int32 Rows, int32 Columns);
    
    // Set room size constraints
    void SetRoomSize(int32 NewMinRoomSize, int32 NewMaxRoomSize);
    
    // Generate rooms in the tile matrix
    void CreateRooms(int32 RoomCount, int32 Seed);
    
    // Print tile matrix to log (debug)
    void PrintDebugTileMap() const;
    
    // Project tile matrix to world coordinates
    void ProjectTileMapLocationsToWorld(float TileSize, TArray<FVector>& FloorLocations, TArray<FWallSpawnPoint>& WallLocations);
    void ProjectTileMapLocationsToWorld(float TileSize, TArray<FRoom>& Rooms, TArray<FVector>& CorridorFloorTiles, TArray<FWallSpawnPoint>& CorridorWalls);

    // Config for room placement attempts
    int32 MaxRandomAttemptsPerRoom = 1500;
    
    // Is the tile matrix initialized
    bool IsValid() const { return RowsNum > 0 && ColumnsNum > 0; }
    
protected:
    // Tile coordinates (row, column)
    typedef TTuple<int32, int32> Tile;
    
    // Manhattan distance between tiles
    static int32 ManhattanDistance(const Tile& A, const Tile& B);
    
    // Convert a tile to string (for debug)
    FString TileToString(const Tile& InTile) const;
    
private:
    // TileMap dimensions
    int32 RowsNum;
    int32 ColumnsNum;
    
    // Contents of the tile matrix (true = occupied, false = empty)
    TArray<TArray<bool>> TileMap;
    
    // Room size constraints
    int32 MinRoomSize = 5;
    int32 MaxRoomSize = 7;
    
    // Get a random tile in the matrix
    Tile GetRandomTile() const;
    
    // Get neighboring tiles (up, down, left, right)
    TArray<Tile> GetNearbyTiles(const Tile& InTile) const;
    
    // Check if a tile is within the matrix bounds
    bool IsTileInMap(const Tile& InTile) const;
    
    // Check if a tile is occupied
    bool IsTileOccupied(Tile InTile) const;
    
    // Check if all tiles in array are valid and unoccupied
    bool AreTilesValid(const TArray<Tile>& InTiles) const;
    
    // Get tile in each direction (returns false if out of bounds)
    bool GetLeftTile(const Tile& InTile, Tile& LeftTile) const;
    bool GetRightTile(const Tile& InTile, Tile& RightTile) const;
    bool GetUpTile(const Tile& InTile, Tile& UpTile) const;
    bool GetDownTile(const Tile& InTile, Tile& DownTile) const;
    
    // Generate wall spawn points around a tile
    TArray<FWallSpawnPoint> GenerateWallSpawnPointsFromNearbyTiles(const Tile& CenterTile, float TileSize) const;
    
    // Mark a tile as occupied
    void OccupyTile(const Tile& InTile);
    
    // Room expansion methods (try to create a room expanding in different directions)
    bool CreateUpperRightRoomExpansion(const Tile& StartTile, int32 ExpansionCount, TArray<Tile>& RoomTiles) const;
    bool CreateLowerRightRoomExpansion(const Tile& StartTile, int32 ExpansionCount, TArray<Tile>& RoomTiles) const;
    bool CreateUpperLeftRoomExpansion(const Tile& StartTile, int32 ExpansionCount, TArray<Tile>& RoomTiles) const;
    bool CreateLowerLeftRoomExpansion(const Tile& StartTile, int32 ExpansionCount, TArray<Tile>& RoomTiles) const;
    
    // Check if a room can be placed at the given tile
    bool CanPlaceRoomInTileMap(Tile InTile, int32 RoomSize, TArray<Tile>& TilesToOccupy) const;
    
    // Room collection structure
    struct FRoomTileCollection
    {
        TArray<Tile> OccupiedTiles;
        
        FRoomTileCollection() { OccupiedTiles.Empty(); }
        FRoomTileCollection(TArray<Tile> RoomTiles) { OccupiedTiles = RoomTiles; }
    };
    
    // Generated rooms
    TArray<FRoomTileCollection> GeneratedRooms;
    
    // Store newly generated room and connect to previous rooms
    void StoreGeneratedRoom(const FRoomTileCollection& InRoom);
    
    // Connect two rooms with corridors
    void ConnectRooms(const FRoomTileCollection& A, const FRoomTileCollection& B);
    
    // Tile connection (for pathfinding between rooms)
    struct FTileConnection
    {
        Tile Start;
        Tile End;
        
        FTileConnection() : Start(0, 0), End(0, 0) {}
        FTileConnection(const Tile& TileA, const Tile& TileB) : Start(TileA), End(TileB) {}
        
        inline int32 Length() const { return ManhattanDistance(Start, End); }
    };
    
    // Detect corners where walls meet
    void DetectCorners(float TileSize, TArray<FCornerPoint>& OutCorners);
};