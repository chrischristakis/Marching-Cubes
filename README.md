# Marching Cubes

<span>
  <image src="res/demo1.png" width="400px">
  <image src="res/demo2.png" width="400px" height="350px">
</span>

The marching Cubes algorithm simulates an isosurface within a scalar field. In other words, it acts to approximate a surface where points in some 
domain are above a certain value.

This program was written in C++ and OpenGL, and features the following:
 * Partitions vertex data into buffer 'batches' with a dynamic size, allowing for enormous vertex counts
 * Multi-threaded, allowing the visualization of the surface generation in real-time
 * Camera operating on spherical coordinates
 * Ability to alter isovalues, scalar field, and other parameters for marching cubes.
   * TO alter parameters, change MarchingCubes::init(...) call in main cpp, as well as 3 parameter general function to whatever surface you want:
   * <image src="res/info.png" width = "300px">
 
 ## Controls
 * <code>Mouse_Drag</code>: Move camera in 360 deg sphere
 * <code>UP</code>: Change radius of camera (Bring closer)
 * <code>DOWN</code>: Change radius of camera (Bring further)
 
 ## Stuff I learned (Future reference for me)
  1) VBOs have a finite size; better to split data than to compound it in one massive buffer.
  2) You cant make calls to OpenGL in two threads at once, you'd have to switch contexts. Just keep it in one thread, and do data processing in another.
  3) Use lock_guard wrapping a mutex to make it exception safe
  4) Should safely close resources. If you termiante the program while its running, youll notice an exception. THis is because I try to end the thread as a mutex is locked, so some cleanup would be nice.
