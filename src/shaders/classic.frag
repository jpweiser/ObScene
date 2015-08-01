// Fragment shader for multiple lights.

#version 410 core

struct LightProperties {
    bool isEnabled;
    bool isLocal;
    bool isSpot;
    vec3 ambient;
    vec3 color;
    vec3 position;
    vec3 halfVector;
    vec3 coneDirection;
    float spotCosCutoff;
    float spotExponent;
    float constantAttenuation;
    float linearAttenuation;
    float quadraticAttenuation;
};

// the set of lights to apply, per invocation of this shader
const int MAXLIGHTS = 4;
uniform LightProperties Lights[MAXLIGHTS];
uniform sampler2D tex;
uniform bool isTextured;

// material description
in vec3 ambient;
in vec3 diffuse;
in vec3 specular;
in float shininess;

in vec3 Normal;		// normal in eye coordinates
in vec4 Position;	// vertex position in eye coordinates

in vec2 tIndex;

out vec4 FragColor;

void main()
{
    vec3 scatteredLight = vec3(0.0); // or, to a global ambient light
    vec3 reflectedLight = vec3(0.0);
	vec3 eyeDirection;
	vec3 lightDirection;
	vec3 halfVector;
	vec3 myNormal;
	float attenuation = 1.0f;
	float diffuseCoeff;
	float specularCoeff;

    // loop over all the lights
    for (int light = 0; light < MAXLIGHTS; ++light)
	{
        if (! Lights[light].isEnabled)
            continue;

        attenuation = 1.0;

        eyeDirection = normalize(-vec3(Position));	// since we are in eye coordinates
													// eye position is 0,0,0
        // for local lights, compute per-fragment direction,
        // halfVector, and attenuation
        if (Lights[light].isLocal)
		{
		    lightDirection = Lights[light].position - vec3(Position);
            float lightDistance = length(lightDirection);
            lightDirection = normalize(lightDirection);

			attenuation = 1.0 /
				(Lights[light].constantAttenuation
				+ Lights[light].linearAttenuation    * lightDistance
				+ Lights[light].quadraticAttenuation * lightDistance
						                                  * lightDistance);
				if (Lights[light].isSpot)
				{
					vec3 myConeDirection = normalize(Lights[light].coneDirection);
					float spotCos = dot(lightDirection,
										-myConeDirection);
					if (spotCos < Lights[light].spotCosCutoff)
						attenuation = 0.0;
					else
						attenuation *= pow(spotCos,
										   Lights[light].spotExponent);
				}
            halfVector = normalize(lightDirection + eyeDirection);
        }
		else
		// directional light
		{
			lightDirection = normalize(Lights[light].position);
			halfVector = normalize(lightDirection + eyeDirection);
        }

		myNormal = normalize(Normal);
        diffuseCoeff  = max(0.0, dot(myNormal, lightDirection));
        specularCoeff = max(0.0, dot(myNormal, halfVector));

        if (diffuseCoeff == 0.0)
            specularCoeff = 0.0;
        else
            specularCoeff = pow(specularCoeff, shininess); // * Strength;

    vec3 diffuseColor = diffuse;

    if (isTextured) {
      vec4 texColor = texture(tex, tIndex);
      if (texColor.a == 0){
          discard;
      }
        diffuseColor = vec3(texColor);
     }

        // Accumulate all the lights� effects as it interacts with material properties
        scatteredLight += Lights[light].ambient * ambient * attenuation +
                          Lights[light].color * (diffuseCoeff * diffuseColor) * attenuation;
        reflectedLight += Lights[light].color * (specularCoeff * specular) * attenuation;
    //reflectedLight = vec3(0,0,0);
    }

	vec3 rgb = min(scatteredLight + reflectedLight, vec3(1.0));
    FragColor = vec4(rgb, 0.0f);
}
