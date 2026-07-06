#version 330

in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec3 lightDir;
uniform vec4 ambient;
uniform vec3 viewPos;

out vec4 finalColor;

void main()
{
    vec4 texelColor = texture(texture0, fragTexCoord) * colDiffuse * fragColor;
    vec3 normal = normalize(fragNormal);
    vec3 light = normalize(-lightDir);
    float ndotl = max(dot(normal, light), 0.0);
    float lit = ambient.r + (1.0 - ambient.r) * ndotl;

    vec3 viewd = normalize(viewPos - fragPosition);
    vec3 halfd = normalize(light + viewd);
    float spec = pow(max(dot(normal, halfd), 0.0), 16.0) * ndotl;

    vec3 color = texelColor.rgb * lit + vec3(spec) * 0.15;
    finalColor = vec4(color, texelColor.a);
}
