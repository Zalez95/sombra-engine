#version 330 core

// ____ CONSTANTS ____
const float EDGE_THRESHOLD_MIN = 0.0312;
const float EDGE_THRESHOLD_MAX = 0.125;
const float REDUCE_MIN = 1.0 / 128.0;
const float REDUCE_MUL = 1.0 / 8.0;
const float SPAN_MAX = 8.0;


// ____ GLOBAL VARIABLES ____
// Input data in world space from the vertex shader
in vec3 vsPosition;

// Uniform variables
uniform sampler2D uColor;

// Output data
out vec4 glFragColor;


// ____ FUNCTIONS ____
/** Calculates the luminosity of the given color */
float rgbToLuma(vec3 rgb)
{
	return sqrt(dot(rgb, vec3(0.299, 0.587, 0.114)));
}


// ____ MAIN PROGRAM ____
// See https://github.com/McNopper/OpenGL/blob/master/Example42/shader/fxaa.frag.glsl
void main()
{
	vec2 texCoords = (0.5 * vsPosition + 0.5).xy;
	vec2 texOffset = 1.0 / textureSize(uColor, 0);

	vec4 color = texture(uColor, texCoords);
	float lumaCenter = rgbToLuma(color.rgb);

	// Get the luma values of the corner neighbours
	float lumaUpRight	= rgbToLuma( texture(uColor, texCoords + texOffset * vec2( 1.0, 1.0)).rgb );
	float lumaUpLeft	= rgbToLuma( texture(uColor, texCoords + texOffset * vec2(-1.0, 1.0)).rgb );
	float lumaDownRight	= rgbToLuma( texture(uColor, texCoords + texOffset * vec2( 1.0,-1.0)).rgb );
	float lumaDownLeft	= rgbToLuma( texture(uColor, texCoords + texOffset * vec2(-1.0,-1.0)).rgb );

	// Find the maximum and minimum luma values around the current fragment
	float lumaMin = min(lumaCenter, min(min(lumaUpRight, lumaUpLeft), min(lumaDownRight, lumaDownLeft)));
	float lumaMax = max(lumaCenter, max(max(lumaUpRight, lumaUpLeft), max(lumaDownRight, lumaDownLeft)));

	// If the luma variation is lower than a threshold we aren't on an edge, skip
	float lumaRange = lumaMax - lumaMin;
	if (lumaRange <= max(EDGE_THRESHOLD_MIN, lumaMax * EDGE_THRESHOLD_MAX)) {
		glFragColor = color;
		return;
	}

	// Sampling is done along the gradient
	vec2 samplingDirection;
	samplingDirection.x = -((lumaUpRight + lumaUpLeft) - (lumaDownRight + lumaDownLeft));
	samplingDirection.y = ((lumaUpRight + lumaDownRight) - (lumaUpLeft + lumaDownLeft));

	// Sampling step distance depends on the luma: The brighter the sampled texels,
	// the smaller the final sampling step direction. This results, that brighter
	// areas are less blurred/more sharper than dark areas
	float samplingDirectionReduce = max(0.25 * REDUCE_MUL * (lumaUpRight + lumaUpLeft + lumaDownRight * lumaDownLeft), REDUCE_MIN);

	// Factor for norming the sampling direction plus adding the brightness influence
	float minSamplingDirectionFactor = 1.0 / (min(abs(samplingDirection.x), abs(samplingDirection.y)) + samplingDirectionReduce);

	// Calculate final sampling direction vector by reducing, clamping to a
	// range and finally adapting to the texture size
	samplingDirection = clamp(samplingDirection * minSamplingDirectionFactor, vec2(-SPAN_MAX), vec2(SPAN_MAX)) * texOffset;

	// Inner samples on the tab
	vec3 rgbSampleNeg = texture(uColor, texCoords + samplingDirection * (1.0 / 3.0 - 0.5)).rgb;
	vec3 rgbSamplePos = texture(uColor, texCoords + samplingDirection * (2.0 / 3.0 - 0.5)).rgb;
	vec3 rgbTwoTab = 0.5 * (rgbSamplePos + rgbSampleNeg);

	// Outer samples on the tab
	vec3 rgbSampleNegOuter = texture(uColor, texCoords + samplingDirection * (0.0 / 3.0 - 0.5)).rgb;
	vec3 rgbSamplePosOuter = texture(uColor, texCoords + samplingDirection * (3.0 / 3.0 - 0.5)).rgb;
	vec3 rgbFourTab = 0.25 * (rgbSamplePosOuter + rgbSampleNegOuter) + 0.5 * rgbTwoTab;

	// Calculate luma for checking against the minimum and maximum value
	float lumaFourTab = rgbToLuma(rgbFourTab);

	// Chech if the outer samples of the tab are beyond the edge
	if (lumaFourTab < lumaMin || lumaFourTab > lumaMax) {
		glFragColor = vec4(rgbTwoTab, 1.0);
	}
	else {
		glFragColor = vec4(rgbFourTab, 1.0);
	}

	// glFragColor.r = 1.0;
}
