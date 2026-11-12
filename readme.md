# CPS 511 Assignment 1 - 3D Drone Simulator

A 3D drone flight simulator built in C++ using the [Raylib](https://www.raylib.com/) library. The drone features hierarchical transformations, dynamic camera tracking, and animated rotor blades that spin based on movement state.

## Controls

The drone requires a minimum altitude to begin moving horizontally. Use the lift controls to take off before attempting to fly directionally.

* **[ R ]** - Ascend (Lift up / Take off)
* **[ F ]** - Descend (Lift down / Land)
* **[ W ]** - Pitch Forward (Move forward)
* **[ S ]** - Pitch Backward (Move backward)
* **[ A ]** - Roll Left (Move left)
* **[ D ]** - Roll Right (Move right)
* **[ Q ]** - Yaw Left (Spin left)
* **[ E ]** - Yaw Right (Spin right)
* **[ SPACE ]** - Motor Kill-switch (Toggles motors off, causing the drone to fall)

## Build Instructions

This project requires the Raylib library to be installed on your system. 

**Using MinGW (Windows):**
Assuming that raylib was downloaded to the default path and has recently been downloaded. Run the command line below.

```bash
g++ -std=c++17 main.cpp Drone.cpp -o game.exe -I "C:\raylib\raylib\src" -L "C:\raylib\raylib\src" -lraylib -lopengl32 -lgdi32 -lwinmm
```
Then, run the executable:
```bash
game.exe
```
*(Note: If you are using a specific IDE like Visual Studio or VS Code, ensure that the raylib include and library paths are correctly configured in your project settings).*

## Known Limitations

**Floating Point Landing:** Due to float precision, landing checks can occasionally cause minor rotational snapping.
**Drone Does Not Maintain Momentum** When the motor is killed, the drone will not maintain its momentum if it was moving at the time that the motor was killed. 