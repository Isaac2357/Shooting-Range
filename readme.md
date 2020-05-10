
# Shooting Range Project

Graphics Programming Project using OpenGL.

## About

This project consits in a simple app that simulates a 3D shooting range.

## Setup

- Clone the repository to your local machine.

``` bash
git clone REPO_URL
```

- Open eclipse and select your workspace.

The workspace must be the paht folder of the repository you just cloned.

- Configure build.

    1. Go to File/Properties/CBuild/Settings

    2. In the windows select MinGWLinker/Libraries

    3. Add the Libray Search Path (-L) as follows: PATH_REPO/Libraries/lib

    4. Add the following Libraries (-l) :
        - glew32
        - freeglut
        - opengl32
        **Note:** The libraries must be in the exact same order.
    5. Clic Apply and Close
    6. Finally, build the project and run it.

## Contact

- Isaac Cabrera Cortés
- Arturo Chávez
- Marcos Fierros
