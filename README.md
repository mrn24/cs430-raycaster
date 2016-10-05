# cs430-raycaster

The project was to create a raycaster that can eat in a .json file and turn it into a ppm.
I completed this by creating a raycaster designed to iterate through every pixel of a proposed picture, 
and together with camera height and pixel dimentions, translate the real world position in the .json file 
and make a 2d picture representing those coordinates.

The raycaster currently only accounts for spheres and planes. For the spheres it takes in a radius, position and color. 
Planes take in a color, position and a normal vector. After all objects are processed, the ray will do its job and store
the correct colors into an Image structure that then gets printed out in p6 .ppm format.

Usage:
Compile with "make"
./raycast <width> <height> <input.json> <output.json>
