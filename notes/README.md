




flags to use while compiling using gcc : -lglfw3 -lGL -lX11 -lpthread -lXrandr -lXi -ldl
[read here](https://learnopengl.com/Getting-started/Creating-a-window#:~:text=For%20Linux%20users%20compiling%20with%20GCC%2C%20the%20following%20command%20line%20options%20may%20help%20you%20compile%20the%20project%3A%20%2Dlglfw3%20%2DlGL%20%2DlX11%20%2Dlpthread%20%2DlXrandr%20%2DlXi%20%2Dldl.%20Not%20correctly%20linking%20the%20corresponding%20libraries%20will%20generate%20many%20undefined%20reference%20errors.)

GLFW : allows us to create an OpenGL context, define window parameters, and handle user input

GLEW : manages function pointers for OpenGL


## graphics 

in OpenGL everything is rendered in 3d space, 


- 3d coordinates ==> 2d 
- 2d ==> colored pixels 

previous step's output is current step's input 
all steps can run in parallel 

processing cores runs programms i.e., shaders on gpu for each step of pipeline 

vertex = collection of vertices
vertex's data = vertex attributes
vertex stores any data , e.g., 3d position, color value

primitives = render types hints, 
these are passed while calling drawing commands


--------


step(input)
O means optional

vertex shader(single vertex)==>geometry shader(collection of vertices)[O]==>primitive assembly stage(all vertices from the shader)==>rasterization(resulting primitives)==>clipping(pixels on final screen ,fragments)==>fragment shader(clipped fragments)==>alpha test ==>blending 

> *A fragment in OpenGL is all the data required for OpenGL to render a single pixel.*

clipping inc performance by dlting the pixels outside view 

fragment shader,calculates final color of pixel and it contains data of scene(color,shading,lighting,.etc)

alpha test and belnding = it checks depth value of fragment , checks if object is at front or behind ,blends them.discard the pixels which are behind some other object


vertex & fragment shader 
optional : geometry shader(uses default shaders)

