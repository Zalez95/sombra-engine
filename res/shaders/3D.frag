#version 130

// ____ CONSTANTS ____
const int MAX_POINT_LIGHTS = 4;


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
	float	mAmbientIntensity;
	float	mIntensity;
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
	vec3 vs_Position;
	vec3 vs_Normal;
	vec2 vs_UV;
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
vec3 calcPhongReflection(BaseLight light, vec3 lightDirection, vec3 viewDirection)
{
	// Calculate the ambient color
	vec3 ambientColor	= u_Material.mAmbientColor * light.mAmbientIntensity;

	// Calculate the diffuse color
	vec3 diffuseColor	= vec3(0,0,0);
	float diffuseAngle	= dot(lightDirection, vs_Normal);
	if (diffuseAngle > 0) {
		diffuseColor	= u_Material.mDiffuseColor * diffuseAngle;
	}

	// Calculate the specular color
	vec3 specularColor	= vec3(0,0,0);
	vec3 lightReflect	= normalize(reflect(lightDirection, vs_Normal));
	float specularAngle	= dot(viewDirection, lightReflect);
	if (specularAngle > 0) {
		specularColor	= u_Material.mSpecularColor * pow(specularAngle, u_Material.mShininess);
	}

	// Add all the light colors and return
	return ambientColor + light.mIntensity * (diffuseColor + specularColor);
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
	vec3 lightColor		= calcPhongReflection(pointLight.mBaseLight, lightDirection, viewDirection);

	// Calculate the attenuation of the point light
	float attenuation	= pointLight.mAttenuation.mConstant
						+ pointLight.mAttenuation.mLinear * distance
						+ pointLight.mAttenuation.mExponential * pow(distance, 2);

	// Apply the attenuation to the light color and return it
	return lightColor / attenuation;
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
	vec3 lightColor = calcDirectLight();
	gl_FragColor = /*texture2D(u_ColorTexture, vs_UV) */ vec4(lightColor, 1.0f);
}
