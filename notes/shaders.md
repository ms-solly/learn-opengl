- shader is program that runs on gpu

mainly two shaders

- vertex shader
- fragment(pixel) shader

data(cpu)==>gpu==>draw call==>vertex shadeer==>fragment shader

data==>result on screen : rendering pipeline

# vertex shader

tells where you want a vertex on screen


vertex data is stored in vertex buffer, vertex shaders operate on each vertex individually using data passed through vertex atttributes, we can bind buffer data to vertex attribute. 

only the data from the vertex buffer that is explicitly bound to vertex attributes is accessible in the vertex shader

fragment shader computes and decides color of the pixels 


