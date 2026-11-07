#version 330

// Input fragment attributes (from fragment shader)
in vec2 fragTexCoord;
in float height;

// Output fragment color
out vec4 finalColor;

void main()
{
    vec4 darkgreen = vec4(0.0, 0.13, 0.0, 1.0);
    vec4 lightgreen = vec4(0.5, 1.0, 0.5, 1.0);
    // interplate between two colors based on height
    finalColor = mix(darkgreen, lightgreen, height+0.2);
}
