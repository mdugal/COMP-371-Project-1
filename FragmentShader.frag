#version 330 core

//in vec4 gl_FragCoord;
out vec4 color;

void main()
{
	
  // color = vec4(gl_FragCoord.z, 0, 1/(gl_FragCoord.z+1)-1, 1.0f);
  color = vec4(1, 2, 1, 1.0f);
} 