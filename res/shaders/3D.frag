#version 130

// ____ CONSTANTS ____
const int	MAX_POINT_LIGHTS	= 4;
const float	SCREEN_GAMMA		= 2.2;	// Monitor is in sRGB color space
const vec3	SCENE_AMBIENT_COLOR = vec3(0.2, 0.2, 0.2);


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
//in VertexData {
in vec3 vs_Position;
in vec3 vs_Normal;
in vec2 vs_UV;
//} vs_Vertex;

flat in int vs_NumPointLights;
in vec3 vs_PointLightsPositions[MAX_POINT_LIGHTS];

// Uniform variables
uniform Material	u_Material;
uniform sampler2D	u_ColorTexture;
uniform PointLight	u_PointLights[MAX_POINT_LIGHTS];
uniform sampler3D	u_VoxelTexture;

// Output data
//out vec4 gl_FragColor;


// ____ FUNCTION DEFINITIONS ____
vec3 calcBlinnPhongReflection(BaseLight light, vec3 lightDirection, vec3 viewDirection)
{
	// Calculate the ambient color
	vec3 ambientColor	= u_Material.mAmbientColor * SCENE_AMBIENT_COLOR;

	// Calculate the diffuse color
	vec3 diffuseColor	= vec3(0,0,0);
	float diffuseDot	= dot(lightDirection, vs_Normal);
	if (diffuseDot > 0) {
		diffuseColor	= u_Material.mDiffuseColor * light.mDiffuseColor * diffuseDot;
	}

	// Calculate the specular color
	vec3 specularColor	= vec3(0,0,0);
	vec3 halfDirection	= normalize(lightDirection + viewDirection);
	float specularDot	= dot(vs_Normal, halfDirection);
	if (specularDot > 0) {
		float spec		= pow(specularDot, u_Material.mShininess);
		specularColor	= u_Material.mSpecularColor * light.mSpecularColor * spec;
	}

	// Add all the light colors and return
	return ambientColor + diffuseColor + specularColor;
}


vec3 calcPointLight(PointLight pointLight, vec3 pointLightPosition)
{
	// Calculate the view direction (the eye is in the center of the scene)
	vec3 viewDirection	= normalize(-vs_Position);
	
	// Calculate the light direction and distance from the current point
	vec3 lightDirection	= pointLightPosition - vs_Position;
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
	for (int i = 0; i < vs_NumPointLights; ++i) {
		totalLight += calcPointLight(u_PointLights[i], vs_PointLightsPositions[i]);
	}

	return totalLight;
}


// ____ MAIN PROGRAM ____
void main()
{
	vec4 lightColor = vec4(calcDirectLight(), 1.0);
	vec4 texColor	= texture2D(u_ColorTexture, vs_UV);
	gl_FragColor	= pow(/*texColor */ lightColor, vec4(1.0 / SCREEN_GAMMA));	// Gamma correction
}

