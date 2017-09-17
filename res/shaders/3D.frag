#version 330 core

// ____ CONSTANTS ____
const int	MAX_POINT_LIGHTS	= 4;
const float	SCREEN_GAMMA		= 2.2;	// Monitor is in sRGB color space
const vec3	SCENE_AMBIENT_COLOR = vec3(0.2, 0.2, 0.2);
const vec2	INVERT_Y_TEXTURE	 = vec2(1.0, -1.0);


// ____ DATATYPES ____
struct Material
{
	vec3	mAmbientColor;
	vec3	mDiffuseColor;
	vec3	mSpecularColor;
	float	mShininess;
	float	mTransparency;
};

struct BaseLight
{
	vec3	mDiffuseColor;
	vec3	mSpecularColor;
};

struct Attenuation
{
	float	mConstant;
	float	mLinear;
	float	mExponential;
};

struct PointLight
{
	BaseLight	mBaseLight;
	Attenuation mAttenuation;
};


// ____ GLOBAL VARIABLES ____
// Input data in view space from the vertex shader
in VertexData
{
	vec3 mPosition;
	vec3 mNormal;
	vec2 mUV;
} vsVertex;

flat in int vsNumPointLights;
in vec3 vsPointLightsPositions[MAX_POINT_LIGHTS];

// Uniform variables
uniform Material	uMaterial;
uniform sampler2D	uColorTexture;
uniform PointLight	uPointLights[MAX_POINT_LIGHTS];

// Output data
out vec4 glFragColor;


// ____ FUNCTION DEFINITIONS ____
vec3 calcBlinnPhongReflection(BaseLight light, vec3 lightDirection, vec3 viewDirection)
{
	// Calculate the ambient color
	vec3 ambientColor	= uMaterial.mAmbientColor * SCENE_AMBIENT_COLOR;

	// Calculate the diffuse color
	vec3 diffuseColor	= vec3(0,0,0);
	float diffuseDot	= dot(lightDirection, vsVertex.mNormal);
	if (diffuseDot > 0) {
		diffuseColor	= uMaterial.mDiffuseColor * light.mDiffuseColor * diffuseDot;
	}

	// Calculate the specular color
	vec3 specularColor	= vec3(0,0,0);
	vec3 halfDirection	= normalize(lightDirection + viewDirection);
	float specularDot	= dot(vsVertex.mNormal, halfDirection);
	if (specularDot > 0) {
		float spec		= pow(specularDot, uMaterial.mShininess);
		specularColor	= uMaterial.mSpecularColor * light.mSpecularColor * spec;
	}

	// Add all the light colors and return
	return ambientColor + diffuseColor + specularColor;
}


vec3 calcPointLight(PointLight pointLight, vec3 pointLightPosition)
{
	// Calculate the view direction (the eye is in the center of the scene)
	vec3 viewDirection	= normalize(-vsVertex.mPosition);

	// Calculate the light direction and distance from the current point
	vec3 lightDirection	= pointLightPosition - vsVertex.mPosition;
	float distance		= length(lightDirection);
	lightDirection		= normalize(lightDirection);

	// Calculate the direct lighting of the current light with the Phong
	// reflection model
	vec3 lightColor		= calcBlinnPhongReflection(pointLight.mBaseLight, lightDirection, viewDirection);

	// Calculate the attenuation of the point light
	float attenuation	= pointLight.mAttenuation.mConstant
						+ pointLight.mAttenuation.mLinear * distance
						+ pointLight.mAttenuation.mExponential * pow(distance, 2);
	attenuation			= (attenuation != 0)? 1.0 / attenuation : 1.0;

	// Apply the attenuation to the light color and return it
	return attenuation * lightColor;
}


vec3 calcDirectLight()
{
	vec3 totalLight = vec3(0,0,0);
	for (int i = 0; i < vsNumPointLights; ++i) {
		totalLight += calcPointLight(uPointLights[i], vsPointLightsPositions[i]);
	}

	return totalLight;
}


// ____ MAIN PROGRAM ____
void main()
{
	vec4 lightColor = vec4(calcDirectLight(), 1.0);
	vec4 texColor	= texture(uColorTexture, INVERT_Y_TEXTURE * vsVertex.mUV);
	glFragColor	= pow(texColor + lightColor, vec4(1.0 / SCREEN_GAMMA));	// Gamma correction
}
