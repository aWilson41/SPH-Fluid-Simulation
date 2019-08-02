#version 460
#define NUM_BOXES 2
#define MAX_SCENE_BOUNDS 1000.0
#define PI 3.14159265359
#define TWO_PI 6.28318530718
#define ONE_OVER_PI (1.0 / PI)
#define ONE_OVER_2PI (1.0 / TWO_PI)
#define EPSILON 0.0001

// Forward declares
float random(vec3 f);
vec3 randomHemispherePoint(vec3 n, vec2 rand);

// Structs
struct Box
{
    vec3 min;
    vec3 max;
    vec3 color;
};
struct Sphere
{
    vec3 pos;
    float r;
};
struct HitInfo
{
    vec2 lambda;
    int boxIndex;
};

// The image we are actually writing too
layout(binding = 0, rgba32f) uniform image2D framebuffer;

// Camera Uniforms
layout(location = 0) uniform vec3 eye;
layout(location = 1) uniform vec3 viewDir;
layout(location = 2) uniform vec3 ray00;
layout(location = 3) uniform vec3 ray10;
layout(location = 4) uniform vec3 ray01;
layout(location = 5) uniform vec3 ray11;

// Scene Uniforms
layout(location = 6) uniform vec3 lightDir;

// Renderer Uniforms
layout(location = 7) uniform float time;
layout(location = 8) uniform float blendRatio;
layout(location = 9) uniform int numBounces;
layout(location = 10) uniform vec3 bgColor;

// Data being rendered
const Box boxes[] =
{
    /* The ground */
    { vec3(-5.0, -0.1, -5.0), vec3(5.0, 0.0, 5.0), vec3(1.0f, 0.0f, 0.0f) },
    /* Box in the middle */
    { vec3(-0.5, 0.0, -0.5), vec3(0.5, 1.0, 0.5), vec3(0.0f, 1.0f, 0.0f) }
};

ivec2 pix;

vec3 normalForBox(vec3 hit, const Box box)
{
    if (hit.x < box.min.x + EPSILON)
        return vec3(-1.0, 0.0, 0.0);
    else if (hit.x > box.max.x - EPSILON)
        return vec3(1.0, 0.0, 0.0);
    else if (hit.y < box.min.y + EPSILON)
        return vec3(0.0, -1.0, 0.0);
    else if (hit.y > box.max.y - EPSILON)
        return vec3(0.0, 1.0, 0.0);
    else if (hit.z < box.min.z + EPSILON)
        return vec3(0.0, 0.0, -1.0);
    else
        return vec3(0.0, 0.0, 1.0);
}

vec2 intersectBox(vec3 origin, vec3 dir, const Box box)
{
    vec3 tMin = (box.min - origin) / dir;
    vec3 tMax = (box.max - origin) / dir;
    vec3 t1 = min(tMin, tMax);
    vec3 t2 = max(tMin, tMax);
    float tNear = max(max(t1.x, t1.y), t1.z);
    float tFar = min(min(t2.x, t2.y), t2.z);
    return vec2(tNear, tFar);
}
bool intersectBoxes(vec3 origin, vec3 dir, out HitInfo info)
{
    float smallest = MAX_SCENE_BOUNDS;
    bool found = false;
    for (int i = 0; i < NUM_BOXES; i++)
    {
        vec2 lambda = intersectBox(origin, dir, boxes[i]);
        if (lambda.x > 0.0 && lambda.x < lambda.y && lambda.x < smallest)
        {
            info.lambda = lambda;
            info.boxIndex = i;
            smallest = lambda.x;
            found = true;
        }
    }
    return found;
}

// bool intersectSphere(vec3 origin, vec3 dir, const Sphere sphere)
// {
//     vec3 m = origin - sphere.pos;
//     float b = dot(m, ray.dir);
//     float c = dot(m, m) - sphere.radius * sphere.radius;

//     // Exit if r’s origin outside s (c > 0) and r pointing away from s (b > 0) 
//     if (c > 0.0f && b > 0.0f)
//         return false;
//     float discr = b * b - c;

//     // A negative discriminant corresponds to ray missing sphere 
//     if (discr < 0.0f)
//         return false;

//     // Ray now found to intersect sphere, compute smallest t value of intersection
//     float t = -b - std::sqrt(discr);

//     // If t is negative, ray started inside sphere so clamp t to zero 
//     if (t < 0.0f)
//         t = 0.0f;
//     vec3 q = ray.pos + t * ray.dir;
//     return true;
// }
// bool intersectSpheres(vec3 origin, vec3 dir, out HitInfo info)
// {
//     float smallest = MAX_SCENE_BOUNDS;
//     bool found = false;
//     for (int i = 0; i < NUM_SPHERES; i++)
//     {
//         vec2 lambda = intersectSphere(origin, dir, spheres[i]);
//         if (lambda.x > 0.0 && lambda.x < lambda.y && lambda.x < smallest)
//         {
//             info.lambda = lambda;
//             info.boxIndex = i;
//             smallest = lambda.x;
//             found = true;
//         }
//     }
//     return found;
// }

vec2 randvec2(int s)
{
    return vec2(
        random(vec3(pix + ivec2(s), time)),
        random(vec3(pix + ivec2(s), time + 1.1)));
}

vec3 trace(vec3 origin, vec3 dir)
{
    vec3 attenuation = vec3(1.0f);
    for (int bounce = 0; bounce < numBounces; bounce++)
    {
        HitInfo hitInfo;
        if (!intersectBoxes(origin, dir, hitInfo))
        {
            if (bounce == 0)
                return bgColor;
            else
                return attenuation;
        }
        Box boxHit = boxes[hitInfo.boxIndex];

        vec3 point = origin + hitInfo.lambda.x * dir;
        vec3 normal = normalForBox(point, boxHit);
        origin = point + normal * EPSILON;
        dir = randomHemispherePoint(normal, randvec2(bounce));
        attenuation *= 2.0f * dot(dir, normal) * boxHit.color;
    }

    return vec3(0.0f);
}

layout (local_size_x = 16, local_size_y = 8) in;
void main(void)
{
    pix = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(framebuffer);
    if (pix.x >= size.x || pix.y >= size.y)
        return;

    vec2 pos = vec2(pix) / vec2(size.x, size.y);
    vec3 rayDir = normalize(mix(mix(ray00, ray01, pos.y), mix(ray10, ray11, pos.y), pos.x));
    vec3 color = trace(eye, rayDir);
    vec3 prevColor = vec3(0.0f);
    if (blendRatio > 0.0f)
        prevColor = imageLoad(framebuffer, pix).rgb;
    vec3 finalColor = mix(color, prevColor, blendRatio);
    imageStore(framebuffer, pix, vec4(finalColor, 1.0f));
}