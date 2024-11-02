// spotlight.fs
#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

uniform sampler2D texture0;
uniform vec2 screenSize;
uniform vec2 lightPos;
uniform float lightRadius;

void main()
{
    vec4 texColor = texture(texture0, fragTexCoord);

    vec2 pixelPos = fragTexCoord * screenSize;
    float distance = length(pixelPos - lightPos);

    // Soft spotlight effect with smoother edges
    float intensity = clamp(1.0 - (distance / lightRadius), 0.0, 1.0);

    // Apply a quadratic falloff for smoother light dissipation
    intensity = pow(intensity, 2.0);

    // Adjust the final color by blending with the texture color
    finalColor = texColor * vec4(vec3(intensity), 1.0);
}
