#version 330 core
out vec4 FragColor;
//in vec4 vertexColor;
//uniform vec4 ourColor;
in vec4 vertexPosition;
in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D ourTexture;
uniform sampler2D texture;
uniform sampler2D texture2;
uniform float mixFactor;// two texture mix factor


void main()
{
   //FragColor = vec4(ourColor, 1.0);
   //FragColor = vec4(vertexPosition);

   //FragColor = texture(ourTexture, TexCoord) * vec4(ourColor, 1.0);
   FragColor = mix(texture(texture, TexCoord), texture(texture2, TexCoord), mixFactor);
   // reverse the second image.
   //FragColor = mix(texture(texture, TexCoord), texture(texture2, vec2(1.0-TexCoord.x,TexCoord.y)), 0.5);
}