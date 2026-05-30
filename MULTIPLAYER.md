# Multiplayer Implementation Guide — DungeonCraft Pro

This guide explains how DungeonCraft Pro handles multiplayer dungeon generation and how to set up your project for reliable server-authoritative dungeons.

---

## Authority Model Overview

DungeonCraft Pro uses a **server-authoritative, seed-replicated** model:

1. The **server** generates a random (or fixed) `DungeonSeed` and calls `GenerateDungeon()`.
2. The `DungeonSeed` property is marked `Replicated` with a `RepNotify` callback (`OnRep_DungeonSeed`).
3. When the seed replicates to each **client**, `OnRep_DungeonSeed` fires and triggers a local `GenerateDungeon()` call using the same seed.
4. Because generation is deterministic for the same seed, all clients produce an **identical dungeon** without any mesh data being sent over the network.

This approach has zero network bandwidth cost for dungeon geometry.

---

## Setup Checklist

| Step | What to do |
|---|---|
| 1 | Place `ADungeonGenerator` in the level and verify `bReplicates = true` (default). |
| 2 | Only call `GenerateDungeon()` on the server (inside `HasAuthority()` guard). |
| 3 | Ensure Blueprint actors you spawn also have `bReplicates = true` if they need to exist on clients. |
| 4 | Add the `DungeonBP` tag to all spawnable Blueprint actors (needed for `ClearDungeon()`). |
| 5 | Test with a Listen Server or Dedicated Server session in PIE. |

---

## Calling GenerateDungeon in Multiplayer

### C++

```cpp
// Only call on server
if (HasAuthority())
{
    DungeonGenerator->GenerateDungeon();
}
```

### Blueprint

Use a **Server RPC** to trigger generation:

1. Create a Custom Event in a server-side Actor (e.g., GameMode or a Manager Actor).
2. Set the event's **Replicates** to `Run on Server`.
3. Inside the event, call `GenerateDungeon()` on the generator reference.

```
Event ServerRequestGeneration (Server, Reliable)
  → DungeonGenerator->GenerateDungeon()
```

---

## Seed Replication Flow

```
[Server]                        [Client]
  │                               │
  ├─ GenerateDungeon() called      │
  ├─ DungeonSeed = FMath::Rand()   │
  ├─ Generate dungeon locally      │
  ├─ Replicate DungeonSeed ───────►│
  │                               ├─ OnRep_DungeonSeed fires
  │                               ├─ Set RNG with DungeonSeed
  │                               └─ Generate dungeon locally
  │                                  (identical result)
```

If the server uses `bUseFixedSeed = true`, the fixed seed is set before generation and is also replicated. Clients will generate the same fixed layout.

---

## Blueprint Actor Spawning in Multiplayer

Blueprint actors spawned by `FBlueprintSpawnParams` are spawned only on the **server** during `GenerateDungeon()`. For these actors to exist on clients, they must be replicated:

1. Open the Blueprint actor class.
2. In the Class Defaults, set **Replicates = true**.
3. If the actor has moving components or state, also enable **Replicate Movement**.

Actors that are purely visual decorations with no interaction do not need to be replicated — spawn them only on the server and clients will see the static mesh dungeon geometry, which is identical by seed.

> **Note:** The dungeon's floor, wall, and ceiling `AStaticMeshActor` instances are **not replicated**. Each machine independently spawns its own geometry from the shared seed.

---

## Regenerating the Dungeon at Runtime

To regenerate mid-session (e.g., after a floor clear):

```cpp
// Server-side game logic
if (HasAuthority())
{
    Generator->bUseFixedSeed = false; // Use new random seed
    Generator->GenerateDungeon();     // Replicates new seed to clients
}
```

Clients automatically receive the new seed and regenerate locally.

---

## Synchronization Timing

In some cases, clients may not be ready to generate the dungeon immediately when the seed replicates (e.g., during level streaming or slow connections).

**Mitigations:**

1. **Delay generation:** Use a `SetTimer` after `OnRep_DungeonSeed` to give the client time to finish level loading.
2. **Late-join handling:** If a client joins after the dungeon is generated, the `DungeonSeed` is already set and `OnRep_DungeonSeed` will fire once the actor replicates to the client.
3. **Loading screen gating:** Trigger `GenerateDungeon()` on the server only after all clients have reported readiness via a server RPC.

---

## Testing Multiplayer Dungeons

### PIE Multi-Client Test

1. In PIE settings, set **Number of Players** to 2 or more.
2. Set **Net Mode** to `Play As Listen Server`.
3. Press Play.
4. Both clients should see the same dungeon layout.

### Dedicated Server Test

1. Package your project or use the editor dedicated server.
2. Start the server with `-server`.
3. Connect two clients with `-game`.
4. Verify seed in the Output Log matches on server and all clients:
   ```
   DungeonGenerator: Using seed 1842019234 for generation
   ```

---

## Known Multiplayer Limitations

| Limitation | Workaround |
|---|---|
| Static mesh actors are not replicated | By design — each client generates its own geometry. This is the intended pattern. |
| Blueprint actors must be manually set to replicate | Enable `Replicates = true` on each spawnable actor class. |
| `ClearDungeon()` must be called on each machine | Call `ClearDungeon()` as part of your server regeneration flow; clients clear via `OnRep_DungeonSeed`. |
| No built-in late-join level streaming integration | Implement a ready-check system and delay server generation until all clients confirm readiness. |
| No incremental/chunk-based generation | Entire dungeon is generated in one blocking call. Consider async offloading for very large dungeons. |

---

## Future Multiplayer Roadmap

- [ ] Built-in late-join recovery (re-send seed on client connection)
- [ ] Optional ISMC-based geometry for reduced actor overhead in large sessions
- [ ] Async generation support for non-blocking client side builds
- [ ] Per-room streaming for World Partition integration
