
# Shooting Range Project

Graphics Programming Project using OpenGL.

[Repository](https://github.com/Isaac2357/Shooting-Range)

[Video](https://www.youtube.com/watch?v=UQkVrRtsCeI)

## About

This project consits in a simple app that simulates a 3D shooting range.

## Controls

### Translations

- A => LEFT

- D => RIGHT

- W => FORWARD

- S => BACKWARD

### Rotations

- Mouse movements Horizontal - YAW, Vertical - PITCH

### Shot

- Mouse right click

## Topics

- Nagivation

The user can navigate through the space (box) in the following directions: forward, backward, left and right. Also the user can rotate over YAW/PITCH using the mouse.

- Lighting 

The lighting model we use is Phong's model (by pixel). The scenario has one ambient light and three light sources that emulates lamps.

- Collision detection

We handle collisions of the user with the walls and cylinders. Also we use collition detection to know  if a bullet hits cylinders and targets.
The algoritm we use is AABB and its variations.

- Mouse picking

We handle the mouse clicks to know when the user shoots. We calculate the ray direction and based on it the bullet moves through the space.

- Textures

We use textures for the walls, the cylinders and the targets.

## Setup

- Clone the repository to your local machine.

``` bash
git clone REPO_URL
```

- Open eclipse and select your workspace.

The workspace must be the path folder of the repository that you just cloned.

- Configure build.

  - Go to File/Properties/CBuild/Settings
  - In the windows select MinGWLinker/Libraries
  - Add the Libray Search Path (-L) as follows: PATH_REPO/Libraries/lib
  - Add the following Libraries (-l) :
    - glew32
    - freeglut
    - opengl32
    **Note:** The libraries must be in the exact same order.
  - Clic Apply and Close
  - Finally, build the project and run it.

## Contact

- Isaac Cabrera Cortés
- Arturo Chávez
- Marcos Fierros
