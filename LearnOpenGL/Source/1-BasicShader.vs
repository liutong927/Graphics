#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;

//out vec4 vertexColor;
out vec4 vertexPosition; // output vertex postion to fs
uniform vec4 offset;
uniform mat4 transform;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 ourColor;
out vec2 TexCoord;

void main()
{
	//gl_Position = transform*(vec4(aPos.x, aPos.y, aPos.z, 1.0)+offset);

	gl_Position = projection*view*model*(vec4(aPos.x, aPos.y, aPos.z, 1.0)+offset);

	vertexPosition = gl_Position;

   // make triangle upside-down.
   //gl_Position = vec4(aPos.x, -aPos.y, aPos.z, 1.0);
   //vertexColor = vec4(0.5, 0.0, 0.0, 1.0);
   ourColor = aColor;

   TexCoord = aTexCoord;
}