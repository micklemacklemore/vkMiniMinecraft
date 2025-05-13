# TODO

### Progress Video 1

https://github.com/user-attachments/assets/70e80763-3f1c-4eee-a231-5f5287349a77

So far I've implemented a simple chunking system (minecraft blocks are grouped as 16x16 "chunks" which own their own VkBuffers) and textures. Below is an outline of the features I want to have as a starting point. Then I'll move on to some more advanced features. 

## Table of Contents

1. [Procedural Terrain](#procedural-terrain)

   * [Terrain Data Structure](#terrain-data-structure)
   * [Biome Rules](#biome-rules)
   * [Notes](#notes)
2. [Multithreaded Terrain Generation](#multithreaded-terrain-generation)

   * [How to Begin](#how-to-begin)
   * [Thread Communication](#thread-communication)
   * [Shared Memory and Mutexes](#shared-memory-and-mutexes)
3. [Game Engine Tick Function and Player Physics](#game-engine-tick-function-and-player-physics)

   * [The Player Class](#the-player-class)
   * [Input Handling and Movement](#input-handling-and-movement)
   * [Flight Mode](#flight-mode)
   * [Ground Mode](#ground-mode)
   * [Game Loop Integration](#game-loop-integration)
   * [Collision and Camera](#collision-and-camera)
   * [Terrain Collision](#terrain-collision)
   * [Block Interaction](#block-interaction)



## Procedural Terrain

Using noise functions, create a height field for your terrain that represents two distinct biomes: a **rolling grassland hills biome** and a **jagged mountain biome**. Write two separate functions that each take in an `(x, z)` coordinate and return a `y` height value for the respective biome. Then, interpolate between those heights using a third noise function as the `t` value in a LERP.

---

### Terrain Data Structure

The `Terrain` class stores a collection of `Chunk`s. A `Chunk` is a 16 × 256 × 16 section of blocks stored in a contiguous 1D array. Chunks are stored in a map of `(x, z)` to `Chunk`. The X and Z directions are infinite, but the Y range is limited to \[0, 255].

---

### Biome Rules

* Y = 0 to Y = 128: fill with `STONE`
* Y > 128: biome-specific blocks based on height field

#### Grassland Biome

* Fill columns with `DIRT`
* Top block should be `GRASS`

#### Mountain Biome

* Fill columns with more `STONE`
* If height > 200, top block should be `SNOW`

#### Water Filling

* Replace any `EMPTY` blocks in \[128, 138] with `WATER`

---

### Notes

* Use Perlin-based noise like:

  ```cpp
  fractal_noise(abs(Perlin(x, z)))
  ```
* For grassland: Voronoi hill effects (see pages 3–4 of [this paper](https://web.mit.edu/cesium/Public/terrain.pdf))

---

## Multithreaded Terrain Generation

Add multithreading to avoid frame drops during terrain expansion.

### How to Begin

* Every **tick**, check a 5x5 zone (20x20 Chunks) around the `Player`:

  * If a zone **does not exist** in `m_generatedTerrain`, spawn a **BlockTypeWorker** to fill it with height field `BlockType` data
  * If a zone **does exist**, spawn a **VBOWorker** if the chunk lacks VBO data

### Thread Communication

* Use shared memory to pass data between workers and main thread
* For `BlockTypeWorker`s: store `Chunk*` in a `std::vector` or `std::unordered_set`
* For `VBOWorker`s: store a `struct` with 4 `std::vector`s (opaque & transparent vertices + indices) in `Terrain`

### Shared Memory and Mutexes

* Use `std::mutex` or `QMutex`
* Lock the mutex when accessing shared memory and unlock afterward

---

## Game Engine Tick Function and Player Physics

`MyGL` handles all entity updates each frame via the `tick` function from the `Entity` base class.

### The Player Class

Inherits from `Entity`, contains:

* A `Camera` (also an `Entity`)
* Velocity and acceleration
* Handle to `Terrain`

### Input Handling and Movement

Refactor `keyPressEvent` to build an `InputBundle`, then pass it to `Player::processInputs`.

Implement `Player::computePhysics` to simulate acceleration, friction, and gravity.

---

### Flight Mode

* `W`/`S`/`A`/`D`: Move along horizontal vectors
* `E`/`Q`: Move vertically
* `F`: Toggle flight OFF

### Ground Mode

* Movement restricted to XZ plane; Y velocity only added via jump
* `Spacebar`: Jump
* `F`: Toggle flight ON

*In both modes, velocity is reduced slightly every frame (friction/drag).*

---

### Game Loop Integration

Call `Player::tick` in `MyGL::tick`.

Use `QDateTime::currentMSecsSinceEpoch()` to compute delta time between frames.
Store and update this value every frame.

---

### Collision and Camera

Assume the `Player` has a 2-block tall collision box:

* Bottom block: `Player` position
* Top block: `Camera` is 1.5 units above the base

---

### Terrain Collision

Use **grid marching** or **axis-by-axis checking**:

* Shoot rays from the bounding box corners based on velocity
* Use delta time to detect intersection and stop or "slide" accordingly

---

### Block Interaction

* **Left click**: Remove block within 3 units of center of screen
* **Right click**: Place block adjacent to that face (also within 3 units)

